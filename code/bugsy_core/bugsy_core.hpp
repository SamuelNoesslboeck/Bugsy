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
# include "../libs/sylo/types.hpp"

// Local headers
# include "motors.hpp"

// Data IO
# define BUGSY_DEVICE_NAME "bugsy"

// Motor control pins
# define PIN_CHAIN_LEFT_FW 0
# define PIN_CHAIN_LEFT_BW 0
# define PIN_CHAIN_RIGHT_FW 0
# define PIN_CHAIN_RIGHT_BW 0

// Misc
# define PARSE_BUFFER_SIZE 128

// Static variables
namespace bugsy {
    enum class Command {
        TEST = 0,
        PING = 1,
        MOVE = 2
    }; 

    // Static fields
    static MotorController motor_ctrl;

    namespace io {
        // Serials
        static BluetoothSerial bt_serial;

        static char parse_buffer [PARSE_BUFFER_SIZE];

        static void parse_cmd(size_t len);
    }

    namespace remote {
        enum class DataMode {
            NONE = 0,
            BLUETOOTH = 1,
            WIFI_UDP = 2
        };

        static DataMode mode = DataMode::NONE;

        // BT
            static bool is_bluetooth() {
                return (mode == DataMode::BLUETOOTH);
            }
        //

        // WiFi
            static char WIFI_SSID [64] = "";
            static char WIFI_PASSWORD [64] = "";

            static bool is_wifi() {
                return ((u8)mode > 1);
            }

            static bool has_wifi_data() {
                return (WIFI_SSID[0] != 0);
            }
        // 

        // Events
            static void start(DataMode new_mode);

            static void end();

            static void swap_mode(DataMode new_mode) {
                end();
                start(new_mode);
            }

            static void handle();

            static void write(char* buffer, size_t len);
        //
    }
}