# pragma once

# include "movement.hpp"

class Bugsy {
private:
    MoveQueue queue;    

    // Motor controller pins
    u8 pin_chain_left_fw, pin_chain_left_bw, pin_chain_right_fw, pin_chain_right_bw;

    void apply_movement(Movement* move) {
        if ((bool)move->chain_left_dir) {
            analogWrite(this->pin_chain_left_bw, 0);
            analogWrite(this->pin_chain_left_fw, move->chain_left_duty);
        } else {
            analogWrite(this->pin_chain_left_fw, 0);
            analogWrite(this->pin_chain_left_bw, move->chain_left_duty);
        }
    }

public:
    Bugsy(u8 pin_chain_left_fw, u8 pin_chain_left_bw, u8 pin_chain_right_fw, u8 pin_chain_right_bw) 
        : pin_chain_left_fw(pin_chain_left_fw), pin_chain_left_bw(pin_chain_left_bw), pin_chain_right_fw(pin_chain_right_fw), pin_chain_right_bw(pin_chain_right_bw)
    {
        // Output pins for motor controller
        pinMode(this->pin_chain_left_fw, OUTPUT);
        pinMode(this->pin_chain_left_bw, OUTPUT);
        pinMode(this->pin_chain_right_fw, OUTPUT);
        pinMode(this->pin_chain_right_bw, OUTPUT);
    }

    void issue_move(Movement move, MoveDuration duration) {
        this->queue.issue_move(move, duration);
    }
 
    void update() {
        // Update movement queue and apply new movement
        this->apply_movement(
            this->queue.update()
        );
    }
};