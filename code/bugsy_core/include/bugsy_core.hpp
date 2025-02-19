# pragma once

// External libraries
# include <Arduino.h>

// Local libraries
# include <sylo/logging.hpp>
# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include <bugsy/core.hpp>
# include <bugsy/trader.hpp>

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
    # define PIN_UART_TRADER_RX 18
    # define PIN_UART_TRADER_TX 19

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
    // Static fields
    
    /// @brief The currently active remotes, configured in the SETUP phase
    extern bugsy::Remote remotes;
    /// @brief The state of the robot
    extern bugsy::CoreState state;

    /// @brief More important (primary) sensor data, becomes invalid if the state of the trader is `DISCONNECTED`
    extern bugsy::PrimarySensorData primary_sensor_data;
}
