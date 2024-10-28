# pragma once

# include "../libs/sylo/types.hpp"

typedef u32 MoveDuration;

struct Movement {
    Direction chain_left_dir, chain_right_dir;
    u8 chain_left_duty, chain_right_duty;

    Movement(Direction chain_left_dir, Direction chain_right_dir, u8 chain_left_duty, u8 chain_right_duty) 
        : chain_left_dir(chain_left_dir), chain_right_dir(chain_right_dir), chain_left_duty(chain_left_duty), chain_right_duty(chain_right_duty)
    { }

    static Movement none() {
        return Movement(Direction::CW, Direction::CCW, 0, 0); 
    }
};

class MoveQueue {
private:
    Movement current_move;
    
    // Timing
    u32 timestamp;
    MoveDuration duration;

public:
    MoveQueue() 
        : current_move(Movement::none()) 
    { }

    //
        void issue_move(Movement move, MoveDuration duration) {
            this->current_move = move;
            this->timestamp = millis();
            this->duration = duration;
        }

        void reset() {
            this->current_move = Movement::none();
        }
    // 

    u32 lasts_till() {
        return timestamp + duration;
    }

    Movement* update() {
        if (this->lasts_till() < millis()) {
            this->reset();
        }

        return &this->current_move;
    }
};