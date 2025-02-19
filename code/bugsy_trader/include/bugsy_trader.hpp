# pragma once

# include <Arduino.h>

# include <bugsy/core.hpp>
# include <bugsy/trader.hpp>

# include <sylo/components/rotary_encoder.hpp>

/// @brief Maximum size of incomming messages
# define PARSE_BUFFER_SIZE 64

// Baud rates
/// @brief Debug baud rate of the trader MCU
# define BUGSY_TRADER_DEBUG_BAUD 115200

// Pins
    # define PIN_ENCODER_DT 0
    # define PIN_ENCODER_CL 0
    # define PIN_ENCODER_SW 0

    
// 

namespace bugsy_trader {
    /// @brief The curret state of the trader MCU
    extern bugsy::TraderState state;

    namespace core {
        /// @brief Stores the last fetched state of the core MCU
        extern bugsy::CoreState state;

        /// @brief Attempts to reconnect to the core MCU
        void reconnect();

        // Commands
        void test();

        bugsy::CoreState get_state();

        bugsy::CoreState set_trader_status(bugsy::TraderState state);

        char* get_wifi_ssid();
    }

    namespace device {
        
    }

    namespace io {
        extern HardwareSerial* core_serial;
        extern char parse_buffer [PARSE_BUFFER_SIZE];

        // Events
            void setup();

            void handle();
        // 

        void send_cmd_core(bugsy::Command cmd);

        template<typename T>
        void send_obj_core(bugsy::Command cmd, T* obj);

        template<typename T>
        T* recv_obj_core();
    }
}