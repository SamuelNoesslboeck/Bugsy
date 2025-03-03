# pragma once

# include <inttypes.h>

# include <Adafruit_PWMServoDriver.h>
# include <bugsy/core.hpp>
# include <sylo/types.hpp>

/// Everything concering the core MCU of the bugsy robot
namespace bugsy_core {
    // Statics
        /// A constant `Movement`, representing no movement
        static bugsy::Movement MOVEMENT_NONE = { Direction::CCW, Direction::CCW, 0, 0 };
    //

    /// Module for everything concerning movements
    namespace move {
        /// The current movement performed by the Bugsy robot
        extern bugsy::Movement move;
        /// The timestamp when the last movement has been applied
        extern uint32_t stamp;
        /// The duration the current movement is valid, `0` means no movement is currently active
        extern bugsy::MoveDuration duration;

        /// The servo driver board
        extern Adafruit_PWMServoDriver servo_driver;

        /// Setup all the motors and drivers required for movements
        void setup();

        /// The timestamp the current movements last to
        /// @return The timestamp
        uint32_t lasts_until();

        /// Apply the given movement to the driver
        /// @param new_move The new movement to be applied to the pins
        void apply_to_pins(const bugsy::Movement* new_move);

        /// Apply a new movement for the given duration
        /// @param new_move The new movement to be applied
        /// @param duration The duration of the new movement until the failsafe activates
        void apply(const bugsy::Movement* new_move, bugsy::MoveDuration duration);

        /// Writes all movements to the drivers and motors
        /// @return Whether a main movement is currently active or not
        bool update();

        /// Stop all movements currently active
        void stop();
    }
}
