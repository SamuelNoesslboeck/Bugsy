// #######################
// #    BUGSY-CORE IO    #
// #######################
//
// IO struct to send and receive inforamtion from other MCUs in the robot

# pragma once

// External libraries
# include <Arduino.h>
# include "bugsy_core.hpp"

namespace bugsy_core {
    /// ## IO-Module
    ///
    /// The IO module manages communication and data transfer between the MCUs of the robot or other parts of the System
    namespace io {
        // Serials
            /// @brief Serial interface between the core and an external device (Laptop / Computer) connected over USB
            extern HardwareSerial* usb_serial;
            /// @brief Serial interface between the core and trader MCU
            extern HardwareSerial* trader_serial;
            /// @brief Serial interface between the core and the RPi zero
            extern HardwareSerial* rpi_serial;
        // 

        /// General buffer for parsing incomming data
        extern char parse_buffer [PARSE_BUFFER_SIZE];

        /// Whether or not the communication to the trader MCU has been established
        extern bool trader_ready;
        /// Whether or not the communication
        extern bool rpi_ready;

        /// Parses a command for the given `buffer` and writes the output to the given `SystemAddr`
        /// @param remotes The remotes to write the output to
        /// @param buffer The buffer to read the data from
        /// @param len The command length
        void parse_cmd(Remote remotes, const char* buffer, size_t len);

        // Events
            /// @brief SETUP everything concering the IO module, should be called in `setup()`
            void setup();

            /// @brief Handle the inputs received by the serials, should be called in `loop()` 
            void handle();
        //

        // Writing
            /// @brief Write some bytes to the given `SystemAddr`
            /// @param remotes The remotes to write the output to
            /// @param buffer The bytes buffer to write
            /// @param len The length of the information to write
            void write(Remote remotes, const uint8_t* buffer, size_t len);

            /// @brief Write a null-terminated string to the serial
            /// @param remotes The remotes to write the output to
            /// @param buffer The string buffer to write
            void write_str(Remote remotes, const char* buffer);

            /// @brief Write an object to the given `SystemAddr`
            /// @tparam T The type of the object
            /// @param remotes The remotes to write the output to
            /// @param obj A pointer to the object to write
            template<typename T>
            void write_obj(Remote remotes, T* obj);
        // 
    }
}