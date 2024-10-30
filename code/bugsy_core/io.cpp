# include "bugsy_core.hpp"

namespace bugsy_core {
        namespace io {
        void parse_cmd(SystemAddr src, char* buffer, size_t len) {
            if (len == 0) {
                log_errorln("> [ERROR] Invalid command length of 0!");
                return;
            }

            Command cmd = (Command)(buffer[0]);

            if (cmd == Command::TEST) {
                log_infoln("> Test command called!");

            } else if (cmd == Command::PING) {
                u32 time = millis();
                io::write_obj(src, &time);

            } else if (cmd == Command::MOVE) {
                if (len == (sizeof(Movement) + 1)) { 
                    Movement move = *((Movement*)&buffer[1]);
                    bugsy_core::motor_ctrl.issue_move(move, config::MOVE_DUR);

                } else {
                    // ERROR: Invalid movement command
                }

            } else if (cmd == Command::REMOTE_CONFIGURE) {
                if (len < (1 + sizeof(RemoteMode))) {
                    log_error("> [ERROR] Command too short for parsing `RemoteMode`");
                    return;
                }

                RemoteMode new_mode = *(RemoteMode*)(buffer + 1);
                // remote::write

            } else if (cmd == Command::CONFIG_SAVE) {   
                log_info("> Saving configuration ... ");
                config::save();
                log_infoln("done!");


            } else if (cmd == Command::CONFIG_GET_WIFI_SSID) {
                io::write_str(src, config::WIFI_SSID);

            } else if (cmd == Command::CONFIG_SET_WIFI_SSID) {
                if (len == 1) {
                    // No SSID given, reseting SSID
                    config::WIFI_SSID[0] = 0;
                } else {
                    strncpy(config::WIFI_SSID, buffer + 1, WIFI_BUFFER_SIZE - 1);     
                    config::WIFI_SSID[WIFI_BUFFER_SIZE - 1] = 0;                        // Safety terminator
                }

                log_info("> New WIFI SSID set: '");
                log_info(config::WIFI_SSID);
                log_infoln("'");

            } else {
                log_error("> [ERROR] Command not found! ID: ");
                log_errorln((uint8_t)cmd);

            } 
        }

        void setup() {
            trader_serial->begin(BUGSY_TRADER_BAUD);
            trader_serial->setTimeout(5);

            pi_serial->begin(BUGSY_PI_BAUD);
            pi_serial->setTimeout(5);
        }   

        void handle() {
            if (trader_serial->available()) {
                io::parse_cmd(
                    SystemAddr::TRADER,
                    io::parse_buffer,
                    io::trader_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }

            if (pi_serial->available()) {
                io::parse_cmd(
                    SystemAddr::RPI,
                    io::parse_buffer,
                    io::pi_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }
        }

        void write(SystemAddr src, const uint8_t* buffer, size_t len) {
            if ((uint8_t)src | (uint8_t)SystemAddr::TRADER) {
                io::trader_serial->write(buffer, len);
            }

            if ((uint8_t)src | (uint8_t)SystemAddr::RPI) {
                io::pi_serial->write(buffer, len);
            }

            if (remote::mode_has_bt(src)) {
                io::bt_serial.write(buffer, len);
            }
        }
    }
}