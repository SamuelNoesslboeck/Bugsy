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
# include <sylo/logging.hpp>

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "bugsy.hpp"
# include "motors.hpp"

// PINS
    // Motor control pins
    /// @brief Duty pin for moving the left chain forward
    # define PIN_CHAIN_LEFT_FW 23
    /// @brief Duty pin for moving the left chain backwards
    # define PIN_CHAIN_LEFT_BW 27
    /// @brief Duty pin for moving the right chain forward
    # define PIN_CHAIN_RIGHT_FW 26
    /// @brief Duty pin for moving the right chain backward
    # define PIN_CHAIN_RIGHT_BW 25

    // Servo pins
    /// @brief Duty pin for the head Servo
    # define PIN_SERVO_HEAD 0

    // Peripheral
    /// @brief Voltage measurement pin, connected to the battery using a voltage divider
    # define PIN_VOLTAGE_MEAS 0

    // UART connections
    # define PIN_SERIAL_TRADER_RX 18
    # define PIN_SERIAL_TRADER_TX 19

    # define PIN_SERIAL_RPI_RX 0
    # define PIN_SERIAL_RPI_TX 0
//

// EEPROM
/// @brief Amount of bytes used in the EEPROM 
# define EEPROM_SIZE 128

/// @brief EEPROM address of the WiFi SSID
# define EEPROM_ADDR_WIFI_SSID      0x0000
/// @brief EEPROM address of the WiFi Password
# define EEPROM_ADDR_WIFI_PWD       0x0020
/// @brief EEPROM address of the Remote mode configuration
# define EEPROM_ADDR_REMOTE_MODE    0x0040

// Buffers
/// @brief Size of the buffer to parse incomming messages from (including null-terminator)
# define PARSE_BUFFER_SIZE 48
/// @brief Size of the buffers to store WiFi credentails in (including null-terminator)
# define WIFI_BUFFER_SIZE 32

// Baud rates
/// @brief Baud rate used for the debug UART connection on the USB Port
# define UART_CORE_DEBUG_BAUD 115200
/// @brief Baud rate between the core and the Pi
# define UART_PI_BAUD 250000

// Misc
/// @brief Amount of time a movement is held active before running out, working as a kind of failssafe for
# define BUGSY_DEFAULT_MOVE_DUR 200

// Static variables
namespace bugsy_core {
    /// Enum consisting of all the possible remotes, assigning them IDs
    ///
    /// Used as a kind of network address to determine where a piece of data was retrieved from or where it has to be sent 
    enum class Remote : uint8_t {
        /// @brief Errorful `None` address, indicates that something went wrong
        NONE = 0x00,

        /// @brief Bluetooth, the main source of remote configuration
        BLUETOOTH = 0x01,

        /// @brief Lora, as a fast, reliable high-range communication method
        LORA = 0x02,

        // Local
        /// @brief Direct UART connection via the USB port
        USB = 0x04,
        /// @brief Direct UART connection to the Trader MCU
        TRADER = 0x08,
        /// @brief Direct UART connection to the RPi 
        RPI = 0x10,

        // Wifi
        /// @brief Wifi data, transfered by a TCP socket
        WIFI_TCP = 0x20,
        /// @brief Wifi data, transfered using MQTT
        WIFI_MQTT = 0x40,

        /// @brief Any WiFi source (all when sending)
        ANY_WIFI = 0x60,

        /// @brief Communication with the Mod-slot
        MOD = 0x80
    };

    // Static fields
    /// @brief The currently active remotes, configured in the SETUP phase
    extern Remote remotes;
    /// @brief The state of the robot
    extern State state;

    namespace config {
        /// @brief The remote stored in the configuration, not representing the current mode! (see bugsy_core::remotes)
        extern Remote saved_remote_mode;
        /// @brief The current move duration, defaulted to `BUGSY_DEFAULT_MOVE_DUR` until loaded from the configuration or changed
        extern MoveDuration move_dur;

        /// @brief The WIFI SSID used, parsed from the configuration on setup
        extern char wifi_ssid [WIFI_BUFFER_SIZE];
        /// @brief The WIFI Password used, parsed from the configuration on setup
        extern char wifi_password [WIFI_BUFFER_SIZE];

        /// @brief Loads the current configuration from the EEPROM
        void load();

        /// @brief Stores the current configuration in the EEPROM
        void save();
    }

    namespace remote {
        // Bluetooth
            extern BluetoothSerial bt_serial;

            extern bool bt_active;

            bool has_bt(Remote _remotes = remotes);

            void start_bt();

            void stop_bt();
        //

        // WiFi
            extern bool wifi_active;

            bool has_wifi(Remote _remotes = remotes);

            bool has_wifi_data();

            void start_wifi();

            void stop_wifi();
        // 

        // General events
            void configure(Remote new_remotes);

            void stop_all();

            void handle();
        //
    }
}
