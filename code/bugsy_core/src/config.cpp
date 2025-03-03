# include "config.hpp"

// External libraries
# include <EEPROM.h>
# include <bugsy/core.hpp>

// Local headers
# include "motors.hpp"

using bugsy::Configuration;

namespace bugsy_core {
    namespace config {
        void load() {
            // Seting up EEPROM
            EEPROM.begin(sizeof(Configuration));

            EEPROM.readBytes(EEPROM_START_ADDR, &configuration, sizeof(Configuration));
            
            // Load values
            configuration.wifi_ssid[BUGSY_WIFI_CRED_BUFFER_SIZE - 1] = 0;        // Adding null terminator to string for safety reasons
            configuration.wifi_password[BUGSY_WIFI_CRED_BUFFER_SIZE - 1] = 0;    // Adding null terminator to string for safety reasons
        }

        void save() {
            EEPROM.writeBytes(EEPROM_START_ADDR, &configuration, sizeof(Configuration));
        }
    }
}