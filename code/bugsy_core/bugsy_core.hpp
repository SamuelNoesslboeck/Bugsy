// ####################
// ##   BUGSY-CORE   ##
// ####################
//
// Defines all the interfaces and pins used by the core bugsy

# pragma once

// External libraries
# include <Arduino.h>
# include <BLEDevice.h>
# include <BLEUtils.h>
# include <BLEServer.h>

// Local libraries
# include "../libs/bugsy.hpp"
# include "../libs/sylo/logging.hpp"
# include "../libs/sylo/types.hpp"

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "motors.hpp"

// Remotes 
# define BUGSY_DEVICE_NAME "bugsy"

# define BUGSY_BLE_SERVICE_UUID "2b43a459-6485-4362-bc1b-36bebe07925a"
# define BUGSY_BLE_CHARACTER_UUID "624672e0-142a-41be-9d45-4f63f7c0d6e9"

// Motor control pins
# define PIN_CHAIN_LEFT_FW 0
# define PIN_CHAIN_LEFT_BW 0
# define PIN_CHAIN_RIGHT_FW 0
# define PIN_CHAIN_RIGHT_BW 0

// EEPROM
# define EEPROM_SIZE 128

# define EEPROM_ADDR_WIFI_SSID      0x0000
# define EEPROM_ADDR_WIFI_PWD       0x0020
# define EEPROM_ADDR_REMOTE_MODE    0x0040

// Buffer sizes
# define PARSE_BUFFER_SIZE 48
# define WIFI_BUFFER_SIZE 32

// Baud rates
# define BUGSY_CORE_DEBUG_BAUD 115200
# define BUGSY_PI_BAUD 250000

// Misc
# define BUGSY_DEFAULT_MOVE_DUR 100

// Static variables
namespace bugsy_core {
    enum class RemoteMode {
        NONE = 0,

        BLUETOOTH = 0x01,

        // Local system nodes, always active, but can also be command sources!
        TRADER = 0x02,
        RPI = 0x04,

        WIFI_UDP = 0x10,
        WIFI_TCP = 0x20,
        WIFI_MQTT = 0x40,
        WIFI_CAM = 0x80,

        ANY_WIFI = 0xF0
    };

    typedef RemoteMode SystemAddr;

        // Static fields
    static RemoteMode REMOTE_MODE = RemoteMode::NONE;
    static Status STATUS;

    namespace config {
        static RemoteMode SAVED_REMOTE_MODE = RemoteMode::NONE;
        static MoveDuration MOVE_DUR = BUGSY_DEFAULT_MOVE_DUR;

        static char WIFI_SSID [WIFI_BUFFER_SIZE] = "";
        static char WIFI_PASSWORD [WIFI_BUFFER_SIZE] = "";

        static void load();

        static void save();
    }

    namespace remote {
        // BT
            static bool BT_ACTIVE = false;

            static bool mode_has_bt(RemoteMode mode = REMOTE_MODE) {
                return (bool)(((uint8_t)mode) | ((uint8_t)RemoteMode::BLUETOOTH));
            }

            static void start_bt();

            static void stop_bt();
        //

        // WiFi
            static bool WIFI_ACTIVE = false;

            static bool mode_has_wifi(RemoteMode mode = REMOTE_MODE) {
                return (bool)(((uint8_t)mode) | ((uint8_t)RemoteMode::ANY_WIFI));
            }

            static bool has_wifi_data() {
                return (bool)(config::WIFI_SSID[0]);
            }

            static void start_wifi();

            static void stop_wifi();
        // 

        // General events
            static void configure(RemoteMode new_mode);

            static void stop_all() {
                stop_bt();
                stop_wifi();

                REMOTE_MODE = RemoteMode::NONE;
            }

            static void handle();
        //
    }
}
