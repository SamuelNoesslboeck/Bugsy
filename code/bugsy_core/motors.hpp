# pragma once

# include <ESP32Servo.h>

# include "sylo/types.hpp"

/// Everything concering the core MCU of the bugsy robot
namespace bugsy_core {
    /// Movement duration in milliseconds, specifies how long a movement command will be kept alive until it runs out
    typedef uint32_t MoveDuration;

    /// A movement with all the information how to adjust the PWM signals
    struct Movement {
        /// The `Direction` to move the left chain in, `Direction::CW` means forward
        Direction chain_left_dir;
        /// The `Direction` to move the right chain in, `Direction::CW` means forward
        Direction chain_right_dir;
        /// The duty of the left chain motor, `0` means fully off while `0xFF` means fully on
        uint8_t chain_left_duty; 
        /// The duty of the left chain motor, `0` means fully off while `0xFF` means fully on
        uint8_t chain_right_duty;

        /// Create a new `Movement`
        /// @param chain_left_dir The `Direction` to move the left chain in, `Direction::CW` means forward
        /// @param chain_right_dir The `Direction` to move the right chain in, `Direction::CW` means forward
        /// @param chain_left_duty 
        /// @param chain_right_duty 
        Movement(Direction chain_left_dir, Direction chain_right_dir, uint8_t chain_left_duty, uint8_t chain_right_duty);
    };

    /// Helper constant indicating no movement
    const static Movement NO_MOVE = Movement(Direction::CCW, Direction::CCW, 0, 0);

    /// Module for everything concerning movements
    namespace move {
        static Movement move = NO_MOVE;
        static uint32_t stamp;
        static MoveDuration duration;

        Servo servo_head;

        void setup();

        void stop();

        uint32_t lasts_until();

        void apply_to_pins(const Movement* new_move);

        void apply(const Movement* new_move, MoveDuration duration);

        void update();
    }
}
