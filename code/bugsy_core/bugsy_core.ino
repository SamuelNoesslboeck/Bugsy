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
                    move::apply((Movement*)arg_bytes, config::mode_dur);

                } else {
                    // ERROR: Invalid movement command
                }

            } else if (cmd == Command::REMOTE_MODE) {
                io::write_obj(src, &remote_mode);

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
                io::write_str(src, config::wifi_ssid);

            } else if (cmd == Command::CONFIG_SET_WIFI_SSID) {
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
            trader_serial->begin(BUGSY_CORE_TO_TRADER_BAUD);
            trader_serial->setTimeout(5);

            rpi_serial->begin(BUGSY_PI_BAUD);
            rpi_serial->setTimeout(5);
        }   

        void handle() {
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
            if ((uint8_t)addr | (uint8_t)SystemAddr::TRADER) {
                io::trader_serial->write(buffer, len);
            }

            if ((uint8_t)addr | (uint8_t)SystemAddr::RPI) {
                io::rpi_serial->write(buffer, len);
            }

            // Remotes
            if (remote::mode_has_bt(addr)) {
                // BLE CODE
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
            void BLECallbacks::onWrite(BLECharacteristic* character) {
                String value = character->getValue();

                io::parse_cmd(SystemAddr::BLUETOOTH, value.c_str(), value.length());
            }

            void start_bt() {
                BLEDevice::init(BUGSY_DEVICE_NAME);
                ble_server = BLEDevice::createServer();
                services::cmd::service = ble_server->createService(BUGSY_BLE_CMD_SERVICE_UUID);
                services::cmd::rx = services::cmd::service->createCharacteristic(
                    BUGSY_BLE_CMD_RX_UUID,
                    BLECharacteristic::PROPERTY_WRITE
                );
                services::cmd::tx = services::cmd::service->createCharacteristic(
                    BUGSY_BLE_CMD_TX_UUID,
                    BLECharacteristic::PROPERTY_READ
                );

                services::cmd::rx->setCallbacks(new BLECallbacks());

                services::cmd::service->start();

                ble_adv = BLEDevice::getAdvertising();

                ble_adv->addServiceUUID(BUGSY_BLE_CMD_SERVICE_UUID);
                ble_adv->setScanResponse(true);
                ble_adv->setMinPreferred(0x06);
                ble_adv->setMaxPreferred(0x12);

                BLEDevice::startAdvertising();

                bt_active = true;
            }

            void stop_bt() {
                // io::bt_serial.end();
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

            if ((!mode_has_wifi(remote_mode)) && (mode_has_wifi(turn_on))) {
                log_info("| > Setting up WiFi ... ");
                start_wifi();
            }

            remote_mode = mode;
        }

        void handle() {
            if (mode_has_bt()) {
                // Skip
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
    bugsy_core::status = Status::SETUP;

    // Load EEPROM
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

    // Apply remote mode with trader always being activated
    bugsy_core::remote::configure(bugsy_core::config::saved_remote_mode);

    log_info("| > Setting up motor ctrl ... ");
    bugsy_core::move::setup();
    log_infoln("done!");

    log_infoln("> Setup complete!");
    bugsy_core::status = Status::RUNNING;
}

void loop() {
    bugsy_core::io::handle();
    bugsy_core::remote::handle();
    bugsy_core::move::update();
}
