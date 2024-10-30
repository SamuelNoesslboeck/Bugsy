// ####################
// ##   BUGSY-CORE   ##
// ####################
// 
// > Version 0.1.0
//
// Code for the main ESP32 controller of the Bugsy robot!

// External libraries
# include "EEPROM.h"

// Local headers
# include "bugsy_core.hpp"
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
