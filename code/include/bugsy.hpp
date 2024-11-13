// #############################
// #    BUGSY - MAIN HEADER    #
// #############################
//
// > Version: 0.1.1/2024/11/04
//
// General header for commands and interfaces shared between multiple controllers in the system

# pragma once

# include <Arduino.h>

/// @brief Current version of the bugsy software
# define BUGSY_SOFTWARE_VERSION "0.1.0/2024/10/30"

// Remotes
/// @brief Current device name 
# define BUGSY_DEVICE_NAME "bugsy"

// Baud rates
/// @brief Baud rate between the core and the trader
# define BUGSY_CORE_TO_TRADER_BAUD 250000

/// Everything concering the core MCU of the bugsy robot
namespace bugsy_core {
    /// The status of the core microcontroller
    enum class Status : uint8_t {
        /// Helper status for external controllers to signal they have not made connection with the microcontroller yet
        Disconnected = 0x20,
        /// The controller is currently setting up
        Setup = 0x00,
        /// The controller is in standby mode
        Standby = 0x10,
        /// The controller is at full activity and running
        Running = 0x01,
        /// The controller has stopped due to a critical error
        CriticalError = 0xFF
    };

    enum class Command : uint8_t {
        /// Test command for debug purposes
        /// @return The additional bytes given
        Test = 0x00,

        /// Status command mainly for internal communication
        /// @return `0x00` - The current `Status` (see `bugsy_core::Status`)
        GetStatus = 0x01,

        /// Issue a new movement
        /// @param 0x00-0x03 4 byte `Movement` struct, will be parsed and applied directly, every sequence of bytes is valid!
        Move = 0x10,

        /// Internal command to signal that the trader is ready
        SetTraderReady = 0x20,
        /// Get whether the trader is ready or not
        /// @return `0x00` bool value
        IsTraderReady = 0x21,
        /// Internal command to signal that the RPi is ready
        SetRPiReady = 0x22,
        /// Get whether the raspberry pi is ready or not
        IsRPiReady = 0x23,

        /// Returns the current remote mode
        /// @return `0x00` - The current remote mode
        RemoteMode = 0x40,
        /// Reconfigures the remote settings made
        /// @param 0x00 The new `RemoteMode`
        RemoteConfigure = 0x41,

        /// Safe the configuration to the EEPROM
        SaveConfig = 0x80,

        /// Get the current SSID for the WiFi connection
        /// @return The WiFi SSID as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        GetWiFiSSID = 0xA0, 
        /// Set the current SSID for the WiFi connection
        /// @param 0x00-? The WiFi SSID as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        SetWiFiSSID = 0xA1,
        
        /// Get the current password for the WiFi connection
        /// @return The WiFi password as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        GetWiFiPwd = 0xA2,
        /// Set the current password for the WiFi connection
        /// @param 0x00-? The WiFi password as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
        SetWiFiPwd = 0xA3
    }; 

    /// The MAC address of the core controller
    static uint8_t MAC [6] = { 0xB0, 0xa7, 0x32, 0x2D, 0x6F, 0x5A };
}