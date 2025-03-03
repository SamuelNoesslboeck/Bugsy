// ####################
// ##   BUGSY-CORE   ##
// ####################
//
// Code for the main ESP32 controller of the Bugsy robot!

# include <bugsy/core.hpp>
# include <bugsy/defines.hpp>

// Local headers
# include "bugsy_core.hpp"

# include "config.hpp"
# include "io.hpp"
# include "motors.hpp"
# include "remote.hpp"

using bugsy::Configuration;
using bugsy::CoreState;
using bugsy::MoveMode;
using bugsy::PrimarySensorData;
using bugsy::Remote;

// Define global events
namespace bugsy_core {
    Remote remotes = Remote::NONE;
    CoreState state = CoreState::NONE;

    Configuration configuration = {     // Default configuration if loading fails
        /* saved_remote_mode: */    Remote::NONE,
        /* move_dur: */             BUGSY_DEFAULT_MOVE_DUR,
        /* wifi_ssid: */            "",
        /* wifi_pwd: */             ""
    };

    PrimarySensorData primary_sensor_data;

    MoveMode move_mode = MoveMode::EXPLORE;
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
        bugsy_core::state = CoreState::SETUP;

        // Load EEPROM - Config
            log_debug("| > Loading EEPROM ... ");
            bugsy_core::config::load();
            log_debugln("done!");

            // Apply always active remotes
            bugsy_core::configuration.saved_remote_mode = (Remote)(
                (uint8_t)bugsy_core::configuration.saved_remote_mode |
                (uint8_t)Remote::BLUETOOTH |                // Bluetooth being always active for configuration
                (uint8_t)Remote::TRADER |                   // "Enabling" these MCUs will not a have any effect and is 
                (uint8_t)Remote::RPI                        // just done for sake of clarity, as they will always be active
            );

            // Print out configuration to trace
            if (bugsy_core::remote::has_bt(bugsy_core::configuration.saved_remote_mode)) {
                log_traceln("| | > Config Bluetooth enabled!");
            }

            if (bugsy_core::remote::has_wifi(bugsy_core::configuration.saved_remote_mode)) {
                log_traceln("| | > Config WiFi enabled!");
            }
        //

        // Apply remote mode with trader & RPi always being activated
        bugsy_core::remote::configure(bugsy_core::configuration.saved_remote_mode);

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

    // Set state and movement mode
    bugsy_core::state = CoreState::STANDBY;
    bugsy_core::move_mode = MoveMode::EXPLORE;
}

void loop() {
    bugsy_core::io::handle();
    bugsy_core::remote::handle();

    if (bugsy_core::move::update()) {
        bugsy_core::state = CoreState::DRIVING; 
    } else {
        bugsy_core::state = CoreState::STANDBY;
    }
}