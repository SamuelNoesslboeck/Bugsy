# pragma once

# include <bugsy/core.hpp>

# include "bugsy_core.hpp"
# include "motors.hpp"

namespace bugsy_core {
    namespace config {
        /// @brief The remote stored in the configuration, not representing the current mode! (see bugsy_core::remotes)
        extern bugsy::Remote saved_remote_mode;
        /// @brief The current move duration, defaulted to `BUGSY_DEFAULT_MOVE_DUR` until loaded from the configuration or changed
        extern bugsy::MoveDuration move_dur;

        /// @brief The WIFI SSID used, parsed from the configuration on setup
        extern char wifi_ssid [WIFI_BUFFER_SIZE];
        /// @brief The WIFI Password used, parsed from the configuration on setup
        extern char wifi_password [WIFI_BUFFER_SIZE];

        /// @brief Loads the current configuration from the EEPROM
        void load();

        /// @brief Stores the current configuration in the EEPROM
        void save();
    }
}