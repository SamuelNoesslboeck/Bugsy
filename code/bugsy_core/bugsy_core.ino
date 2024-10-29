// ####################
// ##   BUGSY-CORE   ##
// ####################
// 
// > Version 0.1.0
//
// Code for the main ESP32 controller of the Bugsy robot!

// External libraries
# include "BluetoothSerial.h"
# include "EEPROM.h"

// Local libraries
# include "../libs/sylo/logging.hpp"

// Local headers
# include "bugsy_core.hpp"
# include "motors.hpp"

# define LOG_LEVEL LOG_LEVEL_TRACE

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

            Command cmd = (Command)(buffer[0]);

            if (cmd == Command::TEST) {
                log_infoln("> Test command called!");

            } else if (cmd == Command::PING) {
                u32 time = millis();
                io::write_obj(src, &time);

            } else if (cmd == Command::MOVE) {
                if (len == (sizeof(Movement) + 1)) { 
                    Movement move = *((Movement*)&buffer[1]);
                    bugsy_core::motor_ctrl.issue_move(move, config::MOVE_DUR);

                } else {
                    // ERROR: Invalid movement command
                }

            } else if (cmd == Command::REMOTE_CONFIGURE) {
                if (len < (1 + sizeof(RemoteMode))) {
                    log_error("> [ERROR] Command too short for parsing `RemoteMode`");
                    return;
                }

                RemoteMode new_mode = *(RemoteMode*)(buffer + 1);
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
            trader_serial->begin(BUGSY_TRADER_BAUD);
            trader_serial->setTimeout(5);

            pi_serial->begin(BUGSY_PI_BAUD);
            pi_serial->setTimeout(5);
        }   

        void handle() {
            if (trader_serial->available()) {
                io::parse_cmd(
                    SystemAddr::TRADER,
                    io::parse_buffer,
                    io::trader_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }

            if (pi_serial->available()) {
                io::parse_cmd(
                    SystemAddr::RPI,
                    io::parse_buffer,
                    io::pi_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }
        }

        void write(SystemAddr src, const uint8_t* buffer, size_t len) {
            if ((uint8_t)src | (uint8_t)SystemAddr::TRADER) {
                io::trader_serial->write(buffer, len);
            }

            if ((uint8_t)src | (uint8_t)SystemAddr::RPI) {
                io::pi_serial->write(buffer, len);
            }

            if (remote::mode_has_bt(src)) {
                io::bt_serial.write(buffer, len);
            }
        }
    }

    namespace remote {
        // Bluetooth
            void start_bt() {
                io::bt_serial.begin(BUGSY_DEVICE_NAME);
                io::bt_serial.setTimeout(5);
                remote::BT_ACTIVE = true;
            }

            void stop_bt() {
                io::bt_serial.end();
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
            RemoteMode action_req = (RemoteMode)((u8)mode ^ (u8)REMOTE_MODE);

            // Services to turn off
            RemoteMode turn_off = (RemoteMode)((u8)action_req & (u8)REMOTE_MODE);

            // Services to turn on
            RemoteMode turn_on = (RemoteMode)((u8)action_req & (u8)mode);

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
                if (io::bt_serial.available()) {
                    io::parse_cmd(
                        SystemAddr::BLUETOOTH,
                        io::parse_buffer,
                        io::bt_serial.readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                    );
                }
            }
        }
    }
}

using bugsy_core::Status;

void setup() {
    // Start logging and print header
    init_logging(BUGSY_DEBUG_BAUD);

    log_infoln("");
    log_debugln("####################");
    log_debugln("##   BUGSY-CORE   ##");
    log_debugln("####################");
    log_debugln("|");
    log_info("> Version: '");
    log_info(BUGSY_CORE_VERSION);
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
    bugsy_core::motor_ctrl.setup();
    log_infoln("done!");

    log_infoln("> Setup complete!");
    bugsy_core::STATUS = Status::RUNNING;
}

void loop() {
    bugsy_core::io::handle();
    bugsy_core::remote::handle();
    bugsy_core::motor_ctrl.update();
}
