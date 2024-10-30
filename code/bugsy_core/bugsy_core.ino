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
            EEPROM.readString(EEPROM_ADDR_WIFI_SSID, WIFI_SSID, WIFI_BUFFER_SIZE - 1);
            WIFI_SSID[WIFI_BUFFER_SIZE - 1] = 0;

            EEPROM.readString(EEPROM_ADDR_WIFI_PWD, WIFI_PASSWORD, WIFI_BUFFER_SIZE - 1);
            WIFI_PASSWORD[WIFI_BUFFER_SIZE - 1] = 0;

            SAVED_REMOTE_MODE = (RemoteMode)(EEPROM.readByte(EEPROM_ADDR_REMOTE_MODE));
        }

        void save() {
            SAVED_REMOTE_MODE = REMOTE_MODE;

            EEPROM.writeString(EEPROM_ADDR_WIFI_SSID, WIFI_SSID);
            EEPROM.writeString(EEPROM_ADDR_WIFI_PWD, WIFI_PASSWORD);
            EEPROM.writeByte(EEPROM_ADDR_REMOTE_MODE, (uint8_t)SAVED_REMOTE_MODE);
        }
    }

    namespace io {
        void parse_cmd(SystemAddr src, char* buffer, size_t len) {
            if (len == 0) {
                log_errorln("> [ERROR] Invalid command length of 0!");
                return;
            }

            Command cmd = *(Command*)buffer;
            char* arg_bytes = buffer + sizeof(Command);
            size_t cmd_len = len - sizeof(Command);

            if (cmd == Command::TEST) {
                log_infoln("> Test command called!");
                
                // Echo the rest of the command back
                if (cmd_len) {
                    io::write(src, (const uint8_t*)arg_bytes, cmd_len);
                }

            } else if (cmd == Command::PING) {
                uint32_t time = millis();
                io::write_obj(src, &time);

            } else if (cmd == Command::MOVE) {
                if (cmd_len == sizeof(Movement)) { 
                    move::apply((Movement*)arg_bytes, config::MOVE_DUR);

                } else {
                    // ERROR: Invalid movement command
                }

            } else if (cmd == Command::REMOTE_MODE) {
                io::write_obj(src, &REMOTE_MODE);

            } else if (cmd == Command::REMOTE_CONFIGURE) {
                if (cmd_len < sizeof(RemoteMode)) {
                    log_error("> [ERROR] Command too short for parsing `RemoteMode`");
                    return;
                }

                RemoteMode new_mode = *(RemoteMode*)arg_bytes;
                // remote::write

            } else if (cmd == Command::CONFIG_SAVE) {   
                log_info("> Saving configuration ... ");
                config::save();
                log_infoln("done!");


            } else if (cmd == Command::CONFIG_GET_WIFI_SSID) {
                io::write_str(src, config::WIFI_SSID);

            } else if (cmd == Command::CONFIG_SET_WIFI_SSID) {
                if (len == 1) {
                    // No SSID given, reseting SSID
                    config::WIFI_SSID[0] = 0;
                } else {
                    strncpy(config::WIFI_SSID, buffer + 1, WIFI_BUFFER_SIZE - 1);     
                    config::WIFI_SSID[WIFI_BUFFER_SIZE - 1] = 0;                        // Safety terminator
                }

                log_info("> New WIFI SSID set: '");
                log_info(config::WIFI_SSID);
                log_infoln("'");

            } else {
                log_error("> [ERROR] Command not found! ID: ");
                log_errorln((uint8_t)cmd);

            } 
        }

        void setup() {
            TRADER_SERIAL->begin(BUGSY_CORE_TO_TRADER_BAUD);
            TRADER_SERIAL->setTimeout(5);

            RPI_SERIAL->begin(BUGSY_PI_BAUD);
            RPI_SERIAL->setTimeout(5);
        }   

        void handle() {
            if (TRADER_SERIAL->available()) {
                io::parse_cmd(
                    SystemAddr::TRADER,
                    io::PARSE_BUFFER,
                    io::TRADER_SERIAL->readBytes(io::PARSE_BUFFER, PARSE_BUFFER_SIZE)
                );
            }

            if (RPI_SERIAL->available()) {
                io::parse_cmd(
                    SystemAddr::RPI,
                    io::PARSE_BUFFER,
                    io::RPI_SERIAL->readBytes(io::PARSE_BUFFER, PARSE_BUFFER_SIZE)
                );
            }
        }

        void write(SystemAddr addr, const uint8_t* buffer, size_t len) {
            if ((uint8_t)addr | (uint8_t)SystemAddr::TRADER) {
                io::TRADER_SERIAL->write(buffer, len);
            }

            if ((uint8_t)addr | (uint8_t)SystemAddr::RPI) {
                io::RPI_SERIAL->write(buffer, len);
            }

            // Remotes
            if (remote::mode_has_bt(addr)) {
                io::bt_serial.write(buffer, len);
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
            DURATION = 0;
            MOVE = NO_MOVE;
        }

        uint32_t lasts_until() {
            return STAMP + DURATION;
        }

        void apply_to_pins(const Movement* move) {
            if ((bool)move->chain_left_dir) {
                analogWrite(PIN_CHAIN_LEFT_BW, 0);
                analogWrite(PIN_CHAIN_LEFT_FW, move->chain_left_duty);
            } else {
                analogWrite(PIN_CHAIN_LEFT_FW, 0);
                analogWrite(PIN_CHAIN_LEFT_BW, move->chain_left_duty);
            }

            if ((bool)move->chain_right_dir) {
                analogWrite(PIN_CHAIN_RIGHT_BW, 0);
                analogWrite(PIN_CHAIN_RIGHT_FW, move->chain_right_duty);
            } else {
                analogWrite(PIN_CHAIN_RIGHT_FW, 0);
                analogWrite(PIN_CHAIN_RIGHT_BW, move->chain_right_duty);
            }
        }

        void apply(const Movement* move, MoveDuration duration) {
            apply_to_pins(move);

            MOVE = *move;
            DURATION = duration;
            STAMP = millis();
        }

        void update() {
            if (DURATION) {
                if (lasts_until() < millis()) {
                    stop();
                }
            }
        }
    }

    namespace remote {
        // Bluetooth
            void start_bt() {
                // io::bt_serial.begin(BUGSY_DEVICE_NAME);
                // io::bt_serial.setTimeout(5);
                remote::BT_ACTIVE = true;
            }

            void stop_bt() {
                // io::bt_serial.end();
                remote::BT_ACTIVE = false;
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
            RemoteMode action_req = (RemoteMode)((uint8_t)mode ^ (uint8_t)REMOTE_MODE);

            // Services to turn off
            RemoteMode turn_off = (RemoteMode)((uint8_t)action_req & (uint8_t)REMOTE_MODE);

            // Services to turn on
            RemoteMode turn_on = (RemoteMode)((uint8_t)action_req & (uint8_t)mode);

            if (mode_has_bt(turn_on)) {
                log_info("| > Setting up bluetooth serial ... ");
                start_bt();
                log_infoln("done!");

                log_info("| | > Device name: '");
                log_info(BUGSY_DEVICE_NAME);
                log_infoln("'");
            }

            if ((!mode_has_wifi(REMOTE_MODE)) && (mode_has_wifi(turn_on))) {
                log_info("| > Setting up WiFi ... ");
                start_wifi();
            }

            REMOTE_MODE = mode;
        }

        void handle() {
            if (mode_has_bt()) {
                // if (io::bt_serial.available()) {
                //     io::parse_cmd(
                //         SystemAddr::BLUETOOTH,
                //         io::PARSE_BUFFER,
                //         io::bt_serial.readBytes(io::PARSE_BUFFER, PARSE_BUFFER_SIZE)
                //     );
                // }
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
    bugsy_core::STATUS = Status::SETUP;

    // Load EEPROM
    log_debug("| > Loading EEPROM ... ");
    bugsy_core::config::load();
    log_debugln("done!");

    // Print out configuration to trace
    if (bugsy_core::remote::mode_has_bt(bugsy_core::config::SAVED_REMOTE_MODE)) {
        log_traceln("| | > Config bluetooth enabled!");
    }

    if (bugsy_core::remote::mode_has_wifi(bugsy_core::config::SAVED_REMOTE_MODE)) {
        log_traceln("| | > Config WiFi enabled!");
    }

    // Apply remote mode with trader always being activated
    bugsy_core::remote::configure(bugsy_core::config::SAVED_REMOTE_MODE);

    log_info("| > Setting up motor ctrl ... ");
    bugsy_core::move::setup();
    log_infoln("done!");

    log_infoln("> Setup complete!");
    bugsy_core::STATUS = Status::RUNNING;
}

void loop() {
    bugsy_core::io::handle();
    bugsy_core::remote::handle();
    bugsy_core::move::update();
}
