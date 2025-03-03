# pragma once

// External libraries
# include <Arduino.h>

// Local libraries
# include <sylo/logging.hpp>
# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include <bugsy/core.hpp>
# include <bugsy/trader.hpp>

/* PINS */
    /* Motor control pins */
    /// Duty pin for moving the left chain forward
    # define PIN_CHAIN_LEFT_FW 23
    /// Duty pin for moving the left chain backwards
    # define PIN_CHAIN_LEFT_BW 27
    /// Duty pin for moving the right chain forward
    # define PIN_CHAIN_RIGHT_FW 26
    /// Duty pin for moving the right chain backward
    # define PIN_CHAIN_RIGHT_BW 25

    /* Servo pins */
    /// Duty pin for the head Servo
    # define PIN_SERVO_HEAD 0

    /* Peripheral */
    /// Voltage measurement pin, connected to the battery using a voltage divider
    # define PIN_VOLTAGE_MEAS 0

    /* UART connections */
    # define PIN_UART_TRADER_RX 18
    # define PIN_UART_TRADER_TX 19

    # define PIN_SERIAL_RPI_RX 0
    # define PIN_SERIAL_RPI_TX 0
//

/// The I2C address of the servo driver board
# define SERVO_DRIVER_ADDR 0x40

/* EEPROM */
/// Starting address of the EEPROM configuration
# define EEPROM_START_ADDR 0x0000

/* Buffers */
/// Size of the buffer to parse incomming messages from (including null-terminator)
# define PARSE_BUFFER_SIZE 48

/* Baud rates */
/// Baud rate used for the debug UART connection on the USB Port
# define UART_CORE_DEBUG_BAUD 115200
/// Baud rate between the core and the Pi
# define UART_PI_BAUD 250000

/* Misc */
/// Amount of time a movement is held active before running out, working as a kind of failssafe for
# define BUGSY_DEFAULT_MOVE_DUR 200

/// The core MCU of the Bugsy robot
namespace bugsy_core {
    /// The currently active remotes, configured in the SETUP phase
    extern bugsy::Remote remotes;
    /// The state of the robot
    extern bugsy::CoreState state;

    /// The configuration saved in the EEPROM of the core MCU
    extern bugsy::Configuration configuration;

    /// More important (primary) sensor data, becomes invalid if the state of the trader is `DISCONNECTED`
    extern bugsy::PrimarySensorData primary_sensor_data;
    /// Less important (secondary) sensor data, does not become invalid if the state of the trader is `DISCONNECTED`
    extern bugsy::SecondarySensorData secondary_sensor_data;

    /// The current mode in which movement is performed
    extern bugsy::MoveMode move_mode;
}
