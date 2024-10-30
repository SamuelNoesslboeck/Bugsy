# pragma once

# include <Arduino.h>

// Local libraries
# include "../libs/bugsy.hpp"


namespace bugsy_trader {
    namespace io {
        static HardwareSerial* CORE_SERIAL = &Serial1;

        // Events
            void setup();

            void handle();
        // 
    }
}