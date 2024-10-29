# include "bugsy_core.hpp"
# include "motors.hpp"

# include <Arduino.h>

namespace bugsy_core {
    // Movement
        Movement::Movement(Direction chain_left_dir, Direction chain_right_dir, u8 chain_left_duty, u8 chain_right_duty) 
            : chain_left_dir(chain_left_dir), chain_right_dir(chain_right_dir), chain_left_duty(chain_left_duty), chain_right_duty(chain_right_duty)
        { }

        Movement Movement::none() {
            return Movement(Direction::CW, Direction::CCW, 0, 0); 
        }
    //

    // MoveQueue
        MoveQueue::MoveQueue() 
            : current_move(Movement::none()) 
        { }

        // Movement
            void MoveQueue::issue_move(Movement move, MoveDuration duration) {
                this->current_move = move;
                this->timestamp = millis();
                this->duration = duration;
            }

            void MoveQueue::reset() {
                this->current_move = Movement::none();
            }

            Movement* MoveQueue::current() {
                return &this->current_move;
            }
        // 

        u32 MoveQueue::lasts_till() {
            return timestamp + duration;
        }

        Movement* MoveQueue::update() {
            if (this->lasts_till() < millis()) {
                this->reset();
            }

            return &this->current_move;
        }
    //

    // MotorController
        void MotorController::apply_movement(Movement* move) {
            if ((bool)move->chain_left_dir) {
                analogWrite(PIN_CHAIN_LEFT_BW, 0);
                analogWrite(PIN_CHAIN_LEFT_FW, move->chain_left_duty);
            } else {
                analogWrite(PIN_CHAIN_LEFT_FW, 0);
                analogWrite(PIN_CHAIN_LEFT_BW, move->chain_left_duty);
            }

            if ((bool)move->chain_right_dir) {
                analogWrite(PIN_CHAIN_RIGHT_BW, 0);
                analogWrite(PIN_CHAIN_RIGHT_FW, move->chain_right_duty);
            } else {
                analogWrite(PIN_CHAIN_RIGHT_FW, 0);
                analogWrite(PIN_CHAIN_RIGHT_BW, move->chain_right_duty);
            }
        }

        void MotorController::setup() {
            // Output pins for motor controller
            pinMode(PIN_CHAIN_LEFT_FW, OUTPUT);
            pinMode(PIN_CHAIN_LEFT_BW, OUTPUT);
            pinMode(PIN_CHAIN_RIGHT_FW, OUTPUT);
            pinMode(PIN_CHAIN_RIGHT_BW, OUTPUT);
        }

        Movement* MotorController::current_movement() {
            return this->queue.current();
        }

        void MotorController::issue_move(Movement move, MoveDuration duration) {
            this->queue.issue_move(move, duration);
        }
    
        void MotorController::update() {
            // Update movement queue and apply new movement
            this->apply_movement(
                this->queue.update()
            );
        }
    //
}