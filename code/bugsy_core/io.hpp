# pragma once

// External libraries
# include <Arduino.h>
# include "bugsy_core.hpp"

namespace bugsy_core {
    namespace io {
        // Serials
        static HardwareSerial* trader_serial = &Serial1;
        static HardwareSerial* rpi_serial = &Serial2;


        static char parse_buffer [PARSE_BUFFER_SIZE];

        static void parse_cmd(SystemAddr addr, const char* buffer, size_t len);

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
}