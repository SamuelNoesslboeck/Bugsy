# pragma once

// External libraries
# include <BluetoothSerial.h>
# include <bugsy/core.hpp>

// Local headers
# include "bugsy_core.hpp"

namespace bugsy_core {
    namespace remote {
        // Bluetooth
            extern BluetoothSerial bt_serial;

            extern bool bt_active;

            bool has_bt(bugsy::Remote _remotes = remotes);

            void start_bt();

            void stop_bt();
        //

        // WiFi
            extern bool wifi_active;

            bool has_wifi(bugsy::Remote _remotes = remotes);

            bool has_wifi_data();

            void start_wifi();

            void stop_wifi();
        // 

        // General events
            void configure(bugsy::Remote new_remotes);

            void stop_all();

            void handle();
        //
    }
}