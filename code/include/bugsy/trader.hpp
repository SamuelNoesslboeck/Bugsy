# pragma once

# include <inttypes.h>

namespace bugsy {
    enum class TraderState : uint8_t {
        DISCONNECTED = 0x00,

        SETUP = 0x10,
        CONNECTING = 0x11,
        ACTIVE = 0x20,

        ERROR = 0x80
    };

    struct PrimarySensorData {

    };

    struct SecondarySensorData {

    };
}