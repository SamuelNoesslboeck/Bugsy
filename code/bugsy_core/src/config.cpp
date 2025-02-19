# include "config.hpp"

// External libraries
# include <EEPROM.h>
# include <bugsy/core.hpp>

// Local headers
# include "motors.hpp"

namespace bugsy_core {
    namespace config {
        bugsy::Remote saved_remote_mode = bugsy::Remote::NONE;
        bugsy::MoveDuration move_dur = BUGSY_DEFAULT_MOVE_DUR; 

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

            saved_remote_mode = (bugsy::Remote)(EEPROM.readByte(EEPROM_ADDR_REMOTE_MODE));
        }

        void save() {
            saved_remote_mode = remotes;

            EEPROM.writeString(EEPROM_ADDR_WIFI_SSID, wifi_ssid);
            EEPROM.writeString(EEPROM_ADDR_WIFI_PWD, wifi_password);
            EEPROM.writeByte(EEPROM_ADDR_REMOTE_MODE, (uint8_t)saved_remote_mode);
        }
    }
}