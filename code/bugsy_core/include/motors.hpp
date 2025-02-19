# pragma once

# include <inttypes.h>
// # include <ESP32Servo.h>
# include <bugsy/core.hpp>
# include <sylo/types.hpp>

/// Everything concering the core MCU of the bugsy robot
namespace bugsy_core {
    // Statics
        static bugsy::Movement MOVEMENT_NONE = { Direction::CCW, Direction::CCW, 0, 0 };
    //

    /// Module for everything concerning movements
    namespace move {
        extern bugsy::Movement move;
        extern uint32_t stamp;
        extern bugsy::MoveDuration duration;

        // static Servo servo_head;

        void setup();

        void stop();

        uint32_t lasts_until();

        void apply_to_pins(const bugsy::Movement* new_move);

        void apply(const bugsy::Movement* new_move, bugsy::MoveDuration duration);

        bool update();
    }
}
