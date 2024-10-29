# pragma once

# include "../libs/sylo/types.hpp"

namespace bugsy_core {
    typedef u32 MoveDuration;

    struct Movement {
        Direction chain_left_dir, chain_right_dir;
        u8 chain_left_duty, chain_right_duty;

        Movement(Direction chain_left_dir, Direction chain_right_dir, u8 chain_left_duty, u8 chain_right_duty);

        static Movement none();
    };


    class MoveQueue {
    private:
        Movement current_move;
        
        // Timing
        u32 timestamp;
        MoveDuration duration;

    public:
        MoveQueue();

        // Movement
            void issue_move(Movement move, MoveDuration duration);

            void reset();

            Movement* current();
        // 

        u32 lasts_till();

        Movement* update();
    };


    class MotorController {
    private:
        MoveQueue queue;    

        void apply_movement(Movement* move);

    public:
        void setup();

        Movement* current_movement();

        void issue_move(Movement move, MoveDuration duration);
    
        void update();
    };
}
