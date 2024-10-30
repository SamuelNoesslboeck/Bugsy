// ####################
// ##   BUGSY-CORE   ##
// ####################
// >
// > Version 0.1.0
//
// Defines all the interfaces and pins used by the core bugsy

# pragma once

# define BUGSY_CORE_VERSION "0.1.0"

// External libraries
# include <Arduino.h>
# include "BluetoothSerial.h"

// Local libraries
# include "../libs/sylo/logging.hpp"
# include "../libs/sylo/types.hpp"

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "motors.hpp"

// Data IO
# define BUGSY_DEVICE_NAME "bugsy"

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
# define BUGSY_DEBUG_BAUD 115200
# define BUGSY_TRADER_BAUD 115200
# define BUGSY_PI_BAUD 115200

// Misc
# define BUGSY_DEFAULT_MOVE_DUR 100

// Static variables
namespace bugsy_core {
    // Status
        enum class Status {
            SETUP,
            STANDBY,
            RUNNING,
            CRITICAL_ERROR
        };

        static Status STATUS;
    // 

    // Commands
        enum class Command {
            /// @brief Test command for simple feedback
            TEST = 0x00,
            /// @brief Ping command for connectivity check
            PING = 0x01,
            STATUS = 0x02,

            /// @brief Issue a new movement
            MOVE = 0x10,

            TRADER_READY = 0x20,
            RPI_READY = 0x21,

            REMOTE_CONFIGURE = 0x40,

            CONFIG_SAVE = 0x80,

            CONFIG_GET_REMOTE_MODE = 0x90,
            CONFIG_SET_REMOTE_MODE = 0x91,

            /// @brief Get the current SSID for the WiFi connection
            CONFIG_GET_WIFI_SSID = 0xA0, 
            /// @brief Set the current SSID for the WiFi connection
            CONFIG_SET_WIFI_SSID = 0xA1,

            CONFIG_GET_WIFI_PWD = 0xA2,
            CONFIG_SET_WIFI_PWD = 0xA3,
        }; 

        enum class TraderCommand {

        };
    //

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

    static RemoteMode REMOTE_MODE = RemoteMode::NONE;

    // Static fields
    static MotorController motor_ctrl;

    namespace config {
        static RemoteMode SAVED_REMOTE_MODE = RemoteMode::NONE;
        static MoveDuration MOVE_DUR = BUGSY_DEFAULT_MOVE_DUR;

        static char WIFI_SSID [WIFI_BUFFER_SIZE] = "";
        static char WIFI_PASSWORD [WIFI_BUFFER_SIZE] = "";

        static void load();

        static void save();
    }

    namespace io {
        // Serials
        static BluetoothSerial bt_serial;

        static HardwareSerial* trader_serial = &Serial1;
        static HardwareSerial* pi_serial = &Serial2;


        static char parse_buffer [PARSE_BUFFER_SIZE];

        static void parse_cmd(SystemAddr src, const char* buffer, size_t len);

        // Events
            static void setup();

            static void handle();
        //

        // I/O
        static void write(SystemAddr addr, const uint8_t* buffer, size_t len);

        static void write_str(SystemAddr addr, const char* buffer) {
            write(addr, (const uint8_t*)buffer, strlen(buffer) + 1);
        }

        template<typename T>
        static void write_obj(SystemAddr addr, T* obj) {
            write(addr, (const uint8_t*)obj, sizeof(T));
        }
        // 
    }

    namespace remote {
        // BT
            static bool BT_ACTIVE = false;

            static bool mode_has_bt(RemoteMode mode = REMOTE_MODE) {
                return (bool)(((u8)mode) | ((u8)RemoteMode::BLUETOOTH));
            }

            static void start_bt();

            static void stop_bt();
        //

        // WiFi
            static bool WIFI_ACTIVE = false;

            static bool mode_has_wifi(RemoteMode mode = REMOTE_MODE) {
                return (bool)(((u8)mode) | ((u8)RemoteMode::ANY_WIFI));
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
