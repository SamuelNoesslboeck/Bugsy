# pragma once

# include <Arduino.h>

// Local libraries
# include "bugsy.hpp"

// Settings
# define BUGSY_TRADER_STATUS_INTERVAL 1000

// Buffers
# define PARSE_BUFFER_SIZE 64

// Baud rates
# define BUGSY_TRADER_DEBUG_BAUD 115200

// Pins
    # define PIN_ENCODER_DT 0
    # define PIN_ENCODER_CL 0
    # define PIN_ENCODER_SW 0


// 

namespace bugsy_trader {
    namespace core {
        static bugsy_core::Status status = bugsy_core::Status::Disconnected;

        void reconnect();

        // Commands
        void test();

        bugsy_core::Status fetch_status();

        void trader_ready();

        char* get_wifi_ssid();
    }

    namespace device {
        
    }

    namespace io {
        static HardwareSerial* core_serial = &Serial3;

        // Events
            void setup();

            void handle();
        // 

        static char parse_buffer [PARSE_BUFFER_SIZE] = "";

        void send_core(bugsy_core::Command cmd);

        template<typename T>
        T* recv_core();
    }
}