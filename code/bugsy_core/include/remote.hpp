# pragma once

// External libraries
# include <BluetoothSerial.h>
# include <bugsy/core.hpp>

// Local headers
# include "bugsy_core.hpp"

namespace bugsy_core {
    namespace remote {
        // Bluetooth
            /// The bluetooth serial to write the command data over. Bluetooth Classic as the following properties
            /// - High power consumption
            /// - High transfer rates
            /// - Longer range
            extern BluetoothSerial bt_serial;

            /// @brief Whether the Bluetooth is active
            extern bool bt_active;

            /// @brief Whether the given `_remotes` has Bluetooth active, defaults to the global `bugsy_core::remotes` variable
            bool has_bt(bugsy::Remote _remotes = remotes);

            /// @brief Starts the bluetooth serial
            void start_bt();

            /// @brief Stops the bluetooth serial
            void stop_bt(); 
        //

        // WiFi
            /// @brief Whether the WiFi is active
            extern bool wifi_active;

            /// @brief Whether the given `_remotes` has WiFi active, defaults to the global `bugsy_core::remotes` variable
            bool has_wifi(bugsy::Remote _remotes = remotes);

            /// @brief Whether any wifi-data has been set yet
            bool is_wifi_data_set();

            /// @brief Starts the WiFi remote
            bugsy::CoreError start_wifi();
            
            /// @brief Stops the WiFi remote 
            void stop_wifi();
        // 

        // General events
            /// @brief Configures the Bugsy to use the new set of remotes `new_remotes` provided
            void configure(bugsy::Remote new_remotes);

            /// @brief Stops all the remotes 
            void stop_all();

            /// @brief Handles all remotes, should be called in the `loop()` of a project
            void handle();
        //
    }
}