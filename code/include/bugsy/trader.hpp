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
        uint32_t distance_mcs_front;
        uint32_t distance_mcs_back;

        uint32_t distance_mm_front;
        uint32_t distance_mm_back;

        float accel_x;
        float accel_y;
        float accel_z;

        // float gyro_a;
        // float gyro_b;
        // float gyro_c;
    };

    struct SecondarySensorData {
        float temperature;
    };
}