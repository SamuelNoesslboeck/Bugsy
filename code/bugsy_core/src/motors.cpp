# include "motors.hpp"

// External libraries

// Local headers
# include "bugsy_core.hpp"

using bugsy::Movement;
using bugsy::MoveDuration;

namespace bugsy_core {
    namespace move {
        Movement move = MOVEMENT_NONE;
        uint32_t stamp = 0;
        MoveDuration duration = 0;

        bool accelerating = false;
        bool deccelerating = false;

        void setup() {
            // Output pins for motor controller
            pinMode(PIN_CHAIN_LEFT_FW, OUTPUT);
            pinMode(PIN_CHAIN_LEFT_BW, OUTPUT);
            pinMode(PIN_CHAIN_RIGHT_FW, OUTPUT);
            pinMode(PIN_CHAIN_RIGHT_BW, OUTPUT);

            // Write initial values to pins (clean up)
            stop();
        }

        void stop() {
            apply_to_pins(&MOVEMENT_NONE);
            duration = 0;
            move = MOVEMENT_NONE;
        }

        uint32_t lasts_until() {
            return stamp + duration;
        }

        void apply_to_pins(const Movement* new_move) {
            // Selecting channel and duty for direction
            if ((bool)new_move->chain_left_dir) {
                analogWrite(PIN_CHAIN_LEFT_BW, 0);
                analogWrite(PIN_CHAIN_LEFT_FW, new_move->chain_left_duty);
            } else {
                analogWrite(PIN_CHAIN_LEFT_FW, 0);
                analogWrite(PIN_CHAIN_LEFT_BW, new_move->chain_left_duty);
            }
            
            // Selecting channel and duty for direction
            if ((bool)new_move->chain_right_dir) {
                analogWrite(PIN_CHAIN_RIGHT_BW, 0);
                analogWrite(PIN_CHAIN_RIGHT_FW, new_move->chain_right_duty);
            } else {
                analogWrite(PIN_CHAIN_RIGHT_FW, 0);
                analogWrite(PIN_CHAIN_RIGHT_BW, new_move->chain_right_duty);
            }
        }

        void apply(const Movement* new_move, MoveDuration duration) {
            apply_to_pins(new_move);

            move::move = *new_move;
            move::duration = duration;
            stamp = millis();
        }

        bool update() {
            if (move_mode == bugsy::MoveMode::POWER) {
                if (duration) {
                    if (lasts_until() < millis()) {
                        stop();
                    } else {
                        return true;
                    }
                }
            } else if (move_mode == bugsy::MoveMode::EXPLORE) {
                if (duration) {
                    //
                }
            }

            return false;
        }
    }
}