# include <sylo/types.hpp>

# include "bugsy.hpp"
# include "bugsy_core.hpp"
# include "motors.hpp"

namespace bugsy_core {
    Movement::Movement(Direction chain_left_dir, Direction chain_right_dir, uint8_t chain_left_duty, uint8_t chain_right_duty) 
        : chain_left_dir(chain_left_dir), chain_right_dir(chain_right_dir), chain_left_duty(chain_left_duty), chain_right_duty(chain_right_duty)
    { }

    namespace move {
        Movement move = MOVEMENT_NONE;
        uint32_t stamp = 0;
        MoveDuration duration = 0;

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
            if ((bool)new_move->chain_left_dir) {
                analogWrite(PIN_CHAIN_LEFT_BW, 0);
                analogWrite(PIN_CHAIN_LEFT_FW, new_move->chain_left_duty);
            } else {
                analogWrite(PIN_CHAIN_LEFT_FW, 0);
                analogWrite(PIN_CHAIN_LEFT_BW, new_move->chain_left_duty);
            }

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
            if (duration) {
                if (lasts_until() < millis()) {
                    stop();
                } else {
                    return true;
                }
            }

            return false;
        }
    }
}