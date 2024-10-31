// #
// # 
// #
//
// General header for commands and interfaces shared between multiple controllers in the system

# pragma once

# include <Arduino.h>

/// @brief Current version of the bugsy software
# define BUGSY_SOFTWARE_VERSION "0.1.0/2024/10/30"

// Baud rates
/// @brief Baud rate between the core and the trader
# define BUGSY_CORE_TO_TRADER_BAUD 250000

/// Everything concering the core MCU of the bugsy robot
namespace bugsy_core {
    /// The status of the core microcontroller
    enum class Status {
        /// Helper status for external controllers to signal they have not made connection with the microcontroller yet
        DISCONNECTED,
        /// The controller is currently setting up
        SETUP,
        /// The controller is in standby mode
        STANDBY,
        /// The controller is at full activity and running
        RUNNING,
        /// The controller has stopped due to a critical error
        CRITICAL_ERROR
    };

    enum class Command : uint8_t {
        /// Test command for debug purposes
        /// @return The additional bytes given
        TEST = 0x00,

        /// Ping command for connectivity check
        /// @return `0x00-0x03` - The uptime in milliseconds as `uint32_t`
        PING = 0x01,
        /// Status command mainly for internal communication
        /// @return `0x00` - The current `Status` (see `bugsy_core::Status`)
        STATUS = 0x02,

        /// Issue a new movement
        /// @param 0x00-0x03 4 byte `Movement` struct, will be parsed and applied directly, every sequence of bytes is valid!
        MOVE = 0x10,

        /// Internal command to signal that the trader is ready
        TRADER_READY = 0x20,
        /// Internal command to signal that the RPi is ready
        RPI_READY = 0x21,

        /// Returns the current remote mode
        /// @return `0x00` - The current remote mode
        REMOTE_MODE = 0x40,
        /// Reconfigures the remote settings made
        /// @param 0x00 The new `RemoteMode`
        REMOTE_CONFIGURE = 0x41,

        /// Safe the configuration to the EEPROM
        CONFIG_SAVE = 0x80,

        /// Get the current SSID for the WiFi connection
        /// @return The WiFi SSID as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        CONFIG_GET_WIFI_SSID = 0xA0, 
        /// Set the current SSID for the WiFi connection
        /// @param 0x00-? The WiFi SSID as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        CONFIG_SET_WIFI_SSID = 0xA1,
        
        /// Get the current password for the WiFi connection
        /// @return The WiFi password as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        CONFIG_GET_WIFI_PWD = 0xA2,
        /// Set the current password for the WiFi connection
        /// @param 0x00-? The WiFi password as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        CONFIG_SET_WIFI_PWD = 0xA3
    }; 
}