// ####################
// ##   BUGSY-CORE   ##
// ####################
//
// Code for the main ESP32 controller of the Bugsy robot!

// External libraries
# include "EEPROM.h"

// Local headers
# include "bugsy_core.hpp"
# include "io.hpp"
# include "motors.hpp"

// Define global events
namespace bugsy_core {
    namespace config {
        void load() {
            // Seting up EEPROM
            EEPROM.begin(EEPROM_SIZE);

            // Load values
            EEPROM.readString(EEPROM_ADDR_WIFI_SSID, wifi_ssid, WIFI_BUFFER_SIZE - 1);
            wifi_ssid[WIFI_BUFFER_SIZE - 1] = 0;

            EEPROM.readString(EEPROM_ADDR_WIFI_PWD, wifi_password, WIFI_BUFFER_SIZE - 1);
            wifi_password[WIFI_BUFFER_SIZE - 1] = 0;

            saved_remote_mode = (RemoteMode)(EEPROM.readByte(EEPROM_ADDR_REMOTE_MODE));
        }

        void save() {
            saved_remote_mode = remote_mode;

            EEPROM.writeString(EEPROM_ADDR_WIFI_SSID, wifi_ssid);
            EEPROM.writeString(EEPROM_ADDR_WIFI_PWD, wifi_password);
            EEPROM.writeByte(EEPROM_ADDR_REMOTE_MODE, (uint8_t)saved_remote_mode);
        }
    }

    namespace io {
        void parse_cmd(SystemAddr src, const char* buffer, size_t len) {
            if (len == 0) {
                log_errorln("> [ERROR] Invalid command length of 0!");
                return;
            }

            Command cmd = *(Command*)buffer;
            const char* arg_bytes = buffer + sizeof(Command);
            size_t cmd_len = len - sizeof(Command);

            if (cmd == Command::Test) {
                log_infoln("> Test command called!");

                // Echo the rest of the command back
                if (cmd_len) {
                    log_trace("| > Input len: ");
                    log_traceln(len);
                    log_trace("| > Remaining len: ");
                    log_traceln(cmd_len);
                    io::write(src, (const uint8_t*)arg_bytes, cmd_len);
                }

            } else if (cmd == Command::GetStatus) {
                io::write_obj(src, &status);

            } else if (cmd == Command::Move) {
                if (cmd_len == sizeof(Movement)) { 
                    move::apply((Movement*)arg_bytes, config::mode_dur);

                } else {
                    log_trace("> Bad movement command with length: ");
                    log_traceln(cmd_len);
                    // ERROR: Invalid movement command
                }

            } else if (cmd == Command::SetTraderReady) {
                io::trader_ready = true;
                log_infoln("> Trader ready!");

            } else if (cmd == Command::IsTraderReady) {
                io::write_obj(src, &io::trader_ready);

            } else if (cmd == Command::SetRPiReady) {
                io::rpi_ready = true;
                log_infoln("> RPi ready!");

            } else if (cmd == Command::IsRPiReady) {
                io::write_obj(src, &io::rpi_ready);

            } else if (cmd == Command::RemoteMode) {
                io::write_obj(src, &remote_mode);

            } else if (cmd == Command::RemoteConfigure) {
                if (cmd_len < sizeof(RemoteMode)) {
                    log_error("> [ERROR] Command too short for parsing `RemoteMode`");
                    return;
                }

                RemoteMode new_mode = *(RemoteMode*)arg_bytes;
                // remote::write

            } else if (cmd == Command::SaveConfig) {   
                log_info("> Saving configuration ... ");
                config::save();
                log_infoln("done!");


            } else if (cmd == Command::GetWiFiSSID) {
                io::write_str(src, config::wifi_ssid);

            } else if (cmd == Command::SetWiFiSSID) {
                if (len == 1) {
                    // No SSID given, reseting SSID
                    config::wifi_ssid[0] = 0;
                } else {
                    strncpy(config::wifi_ssid, buffer + 1, WIFI_BUFFER_SIZE - 1);     
                    config::wifi_ssid[WIFI_BUFFER_SIZE - 1] = 0;                        // Safety terminator
                }

                log_info("> New WIFI SSID set: '");
                log_info(config::wifi_ssid);
                log_infoln("'");

            } else {
                log_error("> [ERROR] Command not found! ID: ");
                log_errorln((uint8_t)cmd);

            } 
        }

        void setup() {
            trader_serial->begin(BUGSY_CORE_TO_TRADER_BAUD, SERIAL_8N1, PIN_SERIAL_TRADER_RX, PIN_SERIAL_TRADER_TX);
            trader_serial->setTimeout(5);

            rpi_serial->begin(BUGSY_PI_BAUD);
            rpi_serial->setTimeout(5);
        }   

        void handle() {
            if (usb_serial->available()) {
                io::parse_cmd(
                    SystemAddr::USB,
                    io::parse_buffer,
                    io::usb_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }

            if (trader_serial->available()) {
                io::parse_cmd(
                    SystemAddr::TRADER,
                    io::parse_buffer,
                    io::trader_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }

            if (rpi_serial->available()) {
                io::parse_cmd(
                    SystemAddr::RPI,
                    io::parse_buffer,
                    io::rpi_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }
        }

        void write(SystemAddr addr, const uint8_t* buffer, size_t len) {
            if ((uint8_t)addr & (uint8_t)SystemAddr::TRADER) {
                io::trader_serial->write(buffer, len);
            }

            if ((uint8_t)addr & (uint8_t)SystemAddr::RPI) {
                io::rpi_serial->write(buffer, len);
            }

            // Remotes
            if (remote::mode_has_bt(addr)) {
                remote::bt_serial.write(buffer, len);
            }
        }
    }

    // Movement
        Movement::Movement(Direction chain_left_dir, Direction chain_right_dir, uint8_t chain_left_duty, uint8_t chain_right_duty) 
            : chain_left_dir(chain_left_dir), chain_right_dir(chain_right_dir), chain_left_duty(chain_left_duty), chain_right_duty(chain_right_duty)
        { }
    //

    namespace move {
        void setup() {
            // Output pins for motor controller
            pinMode(PIN_CHAIN_LEFT_FW, OUTPUT);
            pinMode(PIN_CHAIN_LEFT_BW, OUTPUT);
            pinMode(PIN_CHAIN_RIGHT_FW, OUTPUT);
            pinMode(PIN_CHAIN_RIGHT_BW, OUTPUT);

            // Write initial values to pins (clean up)
            stop();
        }

        void stop() {
            apply_to_pins(&NO_MOVE);
            duration = 0;
            move = NO_MOVE;
        }

        uint32_t lasts_until() {
            return stamp + duration;
        }

        void apply_to_pins(const Movement* new_move) {
            if ((bool)new_move->chain_left_dir) {
                analogWrite(PIN_CHAIN_LEFT_BW, 0);
                analogWrite(PIN_CHAIN_LEFT_FW, new_move->chain_left_duty);
            } else {
                analogWrite(PIN_CHAIN_LEFT_FW, 0);
                analogWrite(PIN_CHAIN_LEFT_BW, new_move->chain_left_duty);
            }

            if ((bool)new_move->chain_right_dir) {
                analogWrite(PIN_CHAIN_RIGHT_BW, 0);
                analogWrite(PIN_CHAIN_RIGHT_FW, new_move->chain_right_duty);
            } else {
                analogWrite(PIN_CHAIN_RIGHT_FW, 0);
                analogWrite(PIN_CHAIN_RIGHT_BW, new_move->chain_right_duty);
            }
        }

        void apply(const Movement* new_move, MoveDuration duration) {
            apply_to_pins(new_move);

            move = *new_move;
            duration = duration;
            stamp = millis();
        }

        void update() {
            if (duration) {
                if (lasts_until() < millis()) {
                    stop();
                }
            }
        }
    }

    namespace remote {
        // Bluetooth
            void start_bt() {
                bt_serial.begin(BUGSY_DEVICE_NAME);
                bt_serial.setTimeout(5);

                bt_active = true;
            }

            void stop_bt() {
                bt_serial.end();

                bt_active = false;
            }
        // 

        // WiFi
            void start_wifi() {
                // Send msg to pi
            }   

            void stop_wifi() {
                // Send msg to pi
            }
        // 

        void configure(RemoteMode mode) {
            RemoteMode action_req = (RemoteMode)((uint8_t)mode ^ (uint8_t)remote_mode);

            // Services to turn off
            RemoteMode turn_off = (RemoteMode)((uint8_t)action_req & (uint8_t)remote_mode);

            // Services to turn on
            RemoteMode turn_on = (RemoteMode)((uint8_t)action_req & (uint8_t)mode);

            if (mode_has_bt(turn_on)) {
                log_info("| > Starting bluetooth ... ");
                start_bt();
                log_infoln("done!");

                log_info("| | > Device name: '");
                log_info(BUGSY_DEVICE_NAME);
                log_infoln("'");
            }

            // if ((!mode_has_wifi(remote_mode)) && (mode_has_wifi(turn_on))) {
            //     log_info("| > Setting up WiFi ... ");
            //     start_wifi();
            //     log_infoln("done!");
            // }

            remote_mode = mode;
        }

        void handle() {
            if (mode_has_bt()) {
                if (bt_serial.available()) {
                    io::parse_cmd(
                        SystemAddr::BLUETOOTH,
                        io::parse_buffer,
                        bt_serial.readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                    );
                }
            }
        }
    }
}

using bugsy_core::Status;

void setup() {
    // Start logging and print header
    init_logging(BUGSY_CORE_DEBUG_BAUD);

    log_infoln("");
    log_debugln("####################");
    log_debugln("##   BUGSY-CORE   ##");
    log_debugln("####################");
    log_debugln("|");
    log_info("> Bugsy Software Version: '");
    log_info(BUGSY_SOFTWARE_VERSION);
    log_debugln("'");
    log_debugln("|");

    // Set status to setup
    log_debugln("> Running setup ... ");
    bugsy_core::status = Status::Setup;

    // Load EEPROM - Config
        log_debug("| > Loading EEPROM ... ");
        bugsy_core::config::load();
        log_debugln("done!");

        // Print out configuration to trace
        if (bugsy_core::remote::mode_has_bt(bugsy_core::config::saved_remote_mode)) {
            log_traceln("| | > Config bluetooth enabled!");
        }

        if (bugsy_core::remote::mode_has_wifi(bugsy_core::config::saved_remote_mode)) {
            log_traceln("| | > Config WiFi enabled!");
        }

        // Apply remote mode with trader & RPi always being activated
        bugsy_core::remote::configure(bugsy_core::config::saved_remote_mode);

        // WiFi will be enabled later in the RPi connection
        // - Bluetooth already belongs to core layer
    //

    // CORE LAYER
        log_debug("| > Setting up motor ctrl ... ");
        bugsy_core::move::setup();
        log_debugln("done!");
    //

    // TRADER & RPI LAYER
        log_debug("| > Setting up io connections ... ");
        bugsy_core::io::setup();
        log_debugln("done!");
    // 

    log_infoln("> Setup complete!");
    bugsy_core::status = Status::Running;
}

void loop() {
    bugsy_core::io::handle();
    bugsy_core::remote::handle();
    bugsy_core::move::update();
}