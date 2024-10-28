// ####################
// ##   BUGSY-CORE   ##
// ####################
// 
// > Version 0.1.0
//
// Code for the main ESP32 controller of the Bugsy robot!

// External libraries
# include "BluetoothSerial.h"

// Local libraries
# include "../libs/sylo/logging.hpp"

// Local headers
# include "bugsy_core.hpp"

# include "io.hpp"
# include "motors.hpp"

# define LOG_LEVEL LOG_LEVEL_INFO

// Define global events
namespace bugsy {
    namespace io {
        void parse_cmd(size_t len) {
            if (len == 0) {
                log_errorln("> [ERROR] Invalid command length of 0!");
                return;
            }

            Command cmd = (Command)(parse_buffer[0]);

            if (cmd == Command::TEST) {
                log_infoln("> Test command called!");
            } else if (cmd == Command::MOVE) {
                if (len == (sizeof(Movement) + 1)) { 
                    Movement move = *((Movement*)&parse_buffer[1]);
                    bugsy::motor_ctrl.issue_move(move, 200);
                } else {
                    // ERROR: Invalid movement command
                }
            } else {
                log_error("> [ERROR] Command not found! ID: ");
                log_errorln((uint8_t)cmd);
            }
        }
    }

    namespace remote {
        void start(DataMode mode) {
            remote::mode = mode;

            if (is_bluetooth()) {
                log_infoln("> Setting up bluetooth serial ... ");
                log_info("| > Device name: '");
                log_info(BUGSY_DEVICE_NAME);
                log_infoln("'");

                io::bt_serial.begin(BUGSY_DEVICE_NAME);
                io::bt_serial.setTimeout(5);

                log_infoln("| > Done!");
            }
        }

        void end() {
            if (is_bluetooth()) {
                io::bt_serial.end();
            }

            mode = DataMode::NONE;
        }

        void handle() {
            if (is_bluetooth()) {
                if (io::bt_serial.available()) {
                    io::parse_cmd(
                        io::bt_serial.readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                    );
                }
            }
        }
    }
}


void setup() {
    init_logging(115200);

    log_infoln("");
    log_infoln("####################");
    log_infoln("##   BUGSY-CORE   ##");
    log_infoln("####################");
    log_infoln(">");
    log_info("> Version: '");
    log_info(BUGSY_CORE_VERSION);
    log_infoln("'");
    log_infoln(">");

    bugsy::motor_ctrl.setup();

    bugsy::remote::start(bugsy::remote::DataMode::BLUETOOTH);
}

void loop() {
    bugsy::remote::handle();
    bugsy::motor_ctrl.update();
}