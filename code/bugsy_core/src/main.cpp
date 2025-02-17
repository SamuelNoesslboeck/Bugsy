// ####################
// ##   BUGSY-CORE   ##
// ####################
//
// Code for the main ESP32 controller of the Bugsy robot!

// External libraries
# include <EEPROM.h>
# include "esp_bt.h"

// Local headers
# include "bugsy_core.hpp"
# include "io.hpp"
# include "motors.hpp"

using bugsy_core::State;

// Define global events
namespace bugsy_core {
    Remote remotes = Remote::NONE;
    State state = State::NONE;

    namespace config {
        Remote saved_remote_mode = Remote::NONE;
        MoveDuration move_dur = BUGSY_DEFAULT_MOVE_DUR; 

        char wifi_ssid [WIFI_BUFFER_SIZE] = "";
        char wifi_password [WIFI_BUFFER_SIZE] = "";

        void load() {
            // Seting up EEPROM
            EEPROM.begin(EEPROM_SIZE);

            // Load values
            EEPROM.readString(EEPROM_ADDR_WIFI_SSID, wifi_ssid, WIFI_BUFFER_SIZE - 1);
            wifi_ssid[WIFI_BUFFER_SIZE - 1] = 0;        // Adding null terminator to string for safety reasons

            EEPROM.readString(EEPROM_ADDR_WIFI_PWD, wifi_password, WIFI_BUFFER_SIZE - 1);
            wifi_password[WIFI_BUFFER_SIZE - 1] = 0;    // Adding null terminator to string for safety reasons

            saved_remote_mode = (Remote)(EEPROM.readByte(EEPROM_ADDR_REMOTE_MODE));
        }

        void save() {
            saved_remote_mode = remotes;

            EEPROM.writeString(EEPROM_ADDR_WIFI_SSID, wifi_ssid);
            EEPROM.writeString(EEPROM_ADDR_WIFI_PWD, wifi_password);
            EEPROM.writeByte(EEPROM_ADDR_REMOTE_MODE, (uint8_t)saved_remote_mode);
        }
    }

    namespace remote {
        // Bluetooth
            BluetoothSerial bt_serial;

            bool bt_active = false;

            bool has_bt(Remote _remotes) {
                return (bool)(((uint8_t)_remotes) & ((uint8_t)Remote::BLUETOOTH));
            }

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
            bool wifi_active = false;

            bool has_wifi(Remote _remotes) {
                return (bool)(((uint8_t)_remotes) & ((uint8_t)Remote::ANY_WIFI));
            }

            bool has_wifi_data() {
                return (bool)(config::wifi_ssid[0]);
            }

            void start_wifi() {
                // Send msg to pi
            }   

            void stop_wifi() {
                // Send msg to pi
            }
        // 

        void configure(Remote mode) {
            // Calculate differences between currently active modes
                // Find out where differences are between the current mode and the requested one
                Remote action_req = (Remote)((uint8_t)mode ^ (uint8_t)remotes);
                // Services to turn off
                Remote turn_off = (Remote)((uint8_t)action_req & (uint8_t)remotes);
                // Services to turn on
                Remote turn_on = (Remote)((uint8_t)action_req & (uint8_t)mode);
            //

            // Activating
            if (has_bt(turn_on)) {
                log_info("| > Starting bluetooth ... ");
                start_bt();
                log_infoln("done!");

                log_debug("| | > Device name: '");
                log_debug(BUGSY_DEVICE_NAME);
                log_debugln("'");
            }

            // Deactivating
            if (has_bt(turn_off)) {
                log_info("| > Stopping bluetooth ... ");
                stop_bt();
                log_infoln("done!");
            }

            remotes = mode;
        }

        void stop_all() {
            stop_bt();
            stop_wifi();

            remotes = Remote::NONE;
        }

        void handle() {
            if (has_bt()) {
                if (bt_serial.available()) {
                    io::parse_cmd(
                        Remote::BLUETOOTH,
                        io::parse_buffer,
                        bt_serial.readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                    );
                }
            }
        }
    }
}

void setup() {
    // Start logging and print header
    init_logging(UART_CORE_DEBUG_BAUD);

    log_infoln("");
    log_debugln("####################");
    log_debugln("##   BUGSY-CORE   ##");
    log_debugln("####################");
    log_debugln("|");
    log_info("> Bugsy Software Version: '");
    log_info(BUGSY_SOFTWARE_VERSION);
    log_debugln("'");
    log_debugln("|");

    // CORE LAYER
        // Set status to setup
        log_debugln("> Running setup ... ");
        bugsy_core::state = State::SETUP;

        // Load EEPROM - Config
            log_debug("| > Loading EEPROM ... ");
            bugsy_core::config::load();
            log_debugln("done!");

            // Apply always active remotes
            bugsy_core::config::saved_remote_mode = (bugsy_core::Remote)(
                (uint8_t)bugsy_core::config::saved_remote_mode |
                (uint8_t)bugsy_core::Remote::BLUETOOTH |                // Bluetooth being always active for configuration
                (uint8_t)bugsy_core::Remote::TRADER |                   // "Enabling" these MCUs will not a have any effect and is 
                (uint8_t)bugsy_core::Remote::RPI                        // just done for sake of clarity, as they will always be active
            );

            // Print out configuration to trace
            if (bugsy_core::remote::has_bt(bugsy_core::config::saved_remote_mode)) {
                log_traceln("| | > Config Bluetooth enabled!");
            }

            if (bugsy_core::remote::has_wifi(bugsy_core::config::saved_remote_mode)) {
                log_traceln("| | > Config WiFi enabled!");
            }
        //

        // Apply remote mode with trader & RPi always being activated
        bugsy_core::remote::configure(bugsy_core::config::saved_remote_mode);

        // WiFi will be enabled later in the RPi connection
        // - Bluetooth already belongs to core layer

        log_debug("| > Setting up motor ctrl ... ");
        bugsy_core::move::setup();
        log_debugln("done!");
    //

    // TRADER & RPI LAYER
        log_debug("| > Setting up io connections ... ");
        bugsy_core::io::setup();
        log_debugln("done!");
    // 

    log_infoln("> SETUP complete!");

    bugsy_core::state = State::STANDBY;
}

void loop() {
    bugsy_core::io::handle();
    bugsy_core::remote::handle();

    if (bugsy_core::move::update()) {
        bugsy_core::state = State::DRIVING; 
    } else {
        bugsy_core::state = State::STANDBY;
    }
}