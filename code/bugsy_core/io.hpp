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
            static HardwareSerial* usb_serial = &Serial0;
            /// @brief Serial interface between the core and trader MCU
            static HardwareSerial* trader_serial = &Serial1;
            /// @brief Serial interface between the core and the RPi zero
            static HardwareSerial* rpi_serial = &Serial2;
        // 

        /// General buffer for parsing incomming data
        static char parse_buffer [PARSE_BUFFER_SIZE];

        /// Parses a command for the given `buffer` and writes the output to the given `SystemAddr`
        /// @param addr The addr to write the output to
        /// @param buffer The buffer to read the data from
        /// @param len The command length
        static void parse_cmd(SystemAddr addr, const char* buffer, size_t len);

        // Events
            /// @brief Setup everything concering the IO module, should be called in `setup()`
            static void setup();

            /// @brief Handle the inputs received by the serials, should be called in `loop()` 
            static void handle();
        //

        // Writing
            /// @brief Write some bytes to the given `SystemAddr`
            /// @param addr The address to write the information to
            /// @param buffer The bytes buffer to write
            /// @param len The length of the information to write
            static void write(SystemAddr addr, const uint8_t* buffer, size_t len);

            /// @brief Write a null-terminated string to the serial
            /// @param addr The address to write the information to
            /// @param buffer The string buffer to write
            static void write_str(SystemAddr addr, const char* buffer) {
                write(addr, (const uint8_t*)buffer, strlen(buffer) + 1);
            }

            /// @brief Write an object to the given `SystemAddr`
            /// @tparam T The type of the object
            /// @param addr The address to write the information to
            /// @param obj A pointer to the object to write
            template<typename T>
            static void write_obj(SystemAddr addr, T* obj) {
                write(addr, (const uint8_t*)obj, sizeof(T));
            }
        // 
    }
}