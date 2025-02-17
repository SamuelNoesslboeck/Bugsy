# include "bugsy.hpp"
# include "bugsy_core.hpp"
# include "io.hpp"

namespace bugsy_core {
    namespace io {
        HardwareSerial* usb_serial = &Serial;
        HardwareSerial* trader_serial = &Serial1;
        HardwareSerial* rpi_serial = &Serial2;

        char parse_buffer [PARSE_BUFFER_SIZE];

        bool trader_ready = false;
        bool rpi_ready = false;

        void parse_cmd(Remote src, const char* buffer, size_t len) {
            if (len == 0) {
                log_errorln("> [ERROR] Invalid command length of 0!");
                return;
            }

            Command cmd = *(Command*)buffer;
            const char* arg_bytes = buffer + sizeof(Command);
            size_t cmd_len = len - sizeof(Command);

            if (cmd == Command::Test) {
                log_infoln("> Test command called!");

                // Echo the rest of the command back
                if (cmd_len) {
                    log_trace("| > Input len: ");
                    log_traceln(len);
                    log_trace("| > Remaining len: ");
                    log_traceln(cmd_len);
                    io::write(src, (const uint8_t*)arg_bytes, cmd_len);
                }

            } else if (cmd == Command::GetState) {
                io::write_obj(src, &state);

            } else if (cmd == Command::Move) {
                if (cmd_len == sizeof(Movement)) { 
                    move::apply((Movement*)arg_bytes, config::move_dur);
                    bugsy_core::state = State::DRIVING;

                } else {
                    log_trace("> Bad movement command with length: ");
                    log_traceln(cmd_len);
                    // ERROR: Invalid movement command
                }

            } else if (cmd == Command::SetTraderReady) {
                io::trader_ready = true;
                log_infoln("> Trader ready!");

            } else if (cmd == Command::IsTraderReady) {
                io::write_obj(src, &io::trader_ready);

            } else if (cmd == Command::SetRPiReady) {
                io::rpi_ready = true;
                log_infoln("> RPi ready!");

            } else if (cmd == Command::IsRPiReady) {
                io::write_obj(src, &io::rpi_ready);

            } else if (cmd == Command::Remotes) {
                io::write_obj(src, &remotes);

            } else if (cmd == Command::RemoteConfigure) {
                if (cmd_len < sizeof(Remote)) {
                    log_error("> [ERROR] Command too short for parsing `Remote`");
                    return;
                }

                Remote new_mode = *(Remote*)arg_bytes;
                // remote::write

            } else if (cmd == Command::SaveConfig) {   
                log_info("> Saving configuration ... ");
                config::save();
                log_infoln("done!");


            } else if (cmd == Command::GetWiFiSSID) {
                io::write_str(src, config::wifi_ssid);

            } else if (cmd == Command::SetWiFiSSID) {
                if (len == 1) {
                    // No SSID given, reseting SSID
                    config::wifi_ssid[0] = 0;
                } else {
                    strncpy(config::wifi_ssid, buffer + 1, WIFI_BUFFER_SIZE - 1);     
                    config::wifi_ssid[WIFI_BUFFER_SIZE - 1] = 0;                        // Safety terminator
                }

                log_info("> New WIFI SSID set: '");
                log_info(config::wifi_ssid);
                log_infoln("'");

            } else {
                log_error("> [ERROR] Command not found! ID: ");
                log_errorln((uint8_t)cmd);

            } 
        }

        void setup() {
            trader_serial->begin(BUGSY_CORE_TO_TRADER_BAUD, SERIAL_8N1, PIN_SERIAL_TRADER_RX, PIN_SERIAL_TRADER_TX);
            trader_serial->setTimeout(5);

            rpi_serial->begin(UART_PI_BAUD);
            rpi_serial->setTimeout(5);
        }   

        void handle() {
            if (usb_serial->available()) {
                io::parse_cmd(
                    Remote::USB,
                    io::parse_buffer,
                    io::usb_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }

            if (trader_serial->available()) {
                io::parse_cmd(
                    Remote::TRADER,
                    io::parse_buffer,
                    io::trader_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }

            if (rpi_serial->available()) {
                io::parse_cmd(
                    Remote::RPI,
                    io::parse_buffer,
                    io::rpi_serial->readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                );
            }
        }

        // Writing
            void write(Remote addr, const uint8_t* buffer, size_t len) {
                if ((uint8_t)addr & (uint8_t)Remote::TRADER) {
                    io::trader_serial->write(buffer, len);
                }

                if ((uint8_t)addr & (uint8_t)Remote::RPI) {
                    io::rpi_serial->write(buffer, len);
                }

                // Remotes
                if (remote::has_bt(addr)) {
                    remote::bt_serial.write(buffer, len);
                }
            }

            void write_str(Remote remotes, const char* buffer) {
                write(remotes, (const uint8_t*)buffer, strlen(buffer) + 1);
            }

            template<typename T>
            void write_obj(Remote remotes, T* obj) {
                write(remotes, (const uint8_t*)obj, sizeof(T));
            }
        // 
    }
}