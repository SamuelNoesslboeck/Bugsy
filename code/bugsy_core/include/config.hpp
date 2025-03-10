# pragma once

# include <bugsy/core.hpp>

# include "bugsy_core.hpp"
# include "motors.hpp"

namespace bugsy_core {
    namespace config {
        /// @brief Loads the current configuration from the EEPROM
        void load();

        /// @brief Stores the current configuration in the EEPROM
        void save();
    }
}