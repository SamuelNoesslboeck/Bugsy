// ####################
// ##   BUGSY-CORE   ##
// ####################
//
// Defines all the interfaces and pins used by the core bugsy

# pragma once

// External libraries
# include <Arduino.h>
# include <BluetoothSerial.h>

// Local libraries
# include "bugsy.hpp"
# include "sylo/logging.hpp"
# include "sylo/types.hpp"

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "motors.hpp"

// Remotes 
# define BUGSY_BLE_CMD_SERVICE_UUID "2b43a459-6485-4362-bc1b-36bebe07925a"
# define BUGSY_BLE_CMD_RX_UUID "624672e0-142a-41be-9d45-4f63f7c0d6e9"
# define BUGSY_BLE_CMD_TX_UUID "624672e1-142a-41be-9d45-4f63f7c0d6e9"

// PINS
    // Motor control pins
    # define PIN_CHAIN_LEFT_FW 0
    # define PIN_CHAIN_LEFT_BW 0
    # define PIN_CHAIN_RIGHT_FW 0
    # define PIN_CHAIN_RIGHT_BW 0

    // Servo pins
    # define PIN_SERVO_HEAD 0

    // Peripheral
    # define PIN_VOLTAGE_MEAS 0

    // Serial
    # define PIN_SERIAL_TRADER_RX 18
    # define PIN_SERIAL_TRADER_TX 19

    # define PIN_SERIAL_RPI_RX 0
    # define PIN_SERIAL_RPI_TX 0
//

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
        USB = 0x02,
        TRADER = 0x04,
        RPI = 0x08,

        WIFI_UDP = 0x10,
        WIFI_TCP = 0x20,
        WIFI_MQTT = 0x40,
        WIFI_CAM = 0x80,

        ANY_WIFI = 0xF0
    };

    typedef RemoteMode SystemAddr;

        // Static fields
    static RemoteMode remote_mode = RemoteMode::NONE;
    static Status status;

    namespace config {
        static RemoteMode saved_remote_mode = RemoteMode::NONE;
        static MoveDuration mode_dur = BUGSY_DEFAULT_MOVE_DUR;

        static char wifi_ssid [WIFI_BUFFER_SIZE] = "";
        static char wifi_password [WIFI_BUFFER_SIZE] = "";

        static void load();

        static void save();
    }

    namespace remote {
        // BLE
            static BluetoothSerial bt_serial;

            static bool bt_active = false;

            static bool mode_has_bt(RemoteMode mode = remote_mode) {
                return (bool)(((uint8_t)mode) & ((uint8_t)RemoteMode::BLUETOOTH));
            }

            static void start_bt();

            static void stop_bt();
        //

        // WiFi
            static bool wifi_active = false;

            static bool mode_has_wifi(RemoteMode mode = remote_mode) {
                return (bool)(((uint8_t)mode) & ((uint8_t)RemoteMode::ANY_WIFI));
            }

            static bool has_wifi_data() {
                return (bool)(config::wifi_ssid[0]);
            }

            static void start_wifi();

            static void stop_wifi();
        // 

        // General events
            static void configure(RemoteMode new_mode);

            static void stop_all() {
                stop_bt();
                stop_wifi();

                remote_mode = RemoteMode::NONE;
            }

            static void handle();
        //
    }
}
