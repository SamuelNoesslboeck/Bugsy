# include <bugsy/core.hpp>
# include <bugsy/defines.hpp>
# include <bugsy/trader.hpp>

# include "bugsy_core.hpp"
# include "config.hpp"
# include "io.hpp"
# include "motors.hpp"
# include "remote.hpp"

using bugsy::Command;
using bugsy::CoreState;
using bugsy::Movement;
using bugsy::Remote;
using bugsy::TraderState;

namespace bugsy_core {
    namespace io {
        // Updating the references to the local serials
        HardwareSerial* usb_serial = &Serial;
        HardwareSerial* trader_serial = &Serial1;
        HardwareSerial* rpi_serial = &Serial2;

        char parse_buffer [PARSE_BUFFER_SIZE];

        TraderState trader_state = TraderState::DISCONNECTED;
        unsigned long trader_stamp = 0;
        bool rpi_ready = false;


        void parse_cmd(Remote src, const char* buffer, size_t len) {
            // Check if a valid command length has been provided
            if (len == 0) {
                log_errorln("> [ERROR] Invalid command length of 0!");
                return;
            }

            Command cmd = *(Command*)buffer;
            const char* arg_bytes = buffer + sizeof(Command);
            size_t arg_len = len - sizeof(Command);

            switch (cmd) {
                case Command::Test:
                    log_infoln("> Test command called!");

                    // Echo the rest of the command back
                    if (arg_len) {
                        log_trace("| > Input len: ");
                        log_traceln(len);
                        log_trace("| > Remaining len: ");
                        log_traceln(arg_len);
                        io::write(src, (const uint8_t*)arg_bytes, arg_len);
                    }

                    break;

                case Command::GetState:
                    io::write_obj(src, &state);
                    break;

                case Command::Move:
                    // Check if the data has a valid size
                    if (arg_len == sizeof(Movement)) { 
                        move::apply((Movement*)arg_bytes, config::move_dur);
                        bugsy_core::state = CoreState::DRIVING;
                    } else {
                        log_trace("> [Command::Move] Bad movement command with length: ");
                        log_traceln(arg_len);
                    }
                    break;

                case Command::SetTraderState:
                    // Check if the data has a valid size
                    if (arg_len != sizeof(bugsy::TraderState)) {
                        log_error("> [Command::SetTraderState] Bad trader state size!");
                        return;
                    }

                    // Update local trader state and send the core state back
                    trader_state = *(bugsy::TraderState*)arg_bytes;
                    io::write_obj(src, &state);
                    trader_stamp = millis();

                    if (trader_state == bugsy::TraderState::ACTIVE) {
                        log_infoln("> Trader active!");
                    }

                    break;

                case Command::GetTraderState:
                    io::write_obj(src, &io::trader_state);
                    break;

                case Command::PublishPrimarySensorData:
                    if (arg_len != sizeof(bugsy::PrimarySensorData)) {
                        log_error("> [Command::PublishPrimarySensorData] Bad sensor data size!");
                        return;
                    }

                    primary_sensor_data = *(bugsy::PrimarySensorData*)arg_bytes;
                    trader_stamp = millis();

                    break;

                case Command::GetPrimarySensorData:
                    io::write_obj(src, &primary_sensor_data);
                    break;

                case Command::SetRPiReady:
                    io::rpi_ready = true;
                    log_infoln("> RPi ready!");
                    break;

                case Command::IsRPiReady:
                    io::write_obj(src, &io::rpi_ready);
                    break;

                case Command::Remotes:
                    io::write_obj(src, &remotes);
                    break;

                case Command::RemoteConfigure: {
                    if (arg_len < sizeof(Remote)) {
                        log_error("> [ERROR] Command too short for parsing `Remote`");
                        return;
                    }
                    Remote new_mode = *(Remote*)arg_bytes;
                    // TODO: Add reconfiguration
                    break;
                }
                case Command::SaveConfig:
                    log_info("> Saving configuration ... ");
                    config::save();
                    log_infoln("done!");
                    break;

                case Command::GetWiFiSSID:
                    io::write_str(src, config::wifi_ssid);
                    break;

                case Command::SetWiFiSSID:
                    if (len == 1) {
                        config::wifi_ssid[0] = 0;
                    } else {
                        strncpy(config::wifi_ssid, buffer + 1, WIFI_BUFFER_SIZE - 1);
                        config::wifi_ssid[WIFI_BUFFER_SIZE - 1] = 0;
                    }
                    log_info("> New WIFI SSID set: '");
                    log_info(config::wifi_ssid);
                    log_infoln("'");
                    break;

                default:
                    log_error("> [ERROR] Command not found! ID: ");
                    log_errorln((uint8_t)cmd);
                    break;
            }
        }

        void setup() {
            trader_serial->begin(BUGSY_CORE_TO_TRADER_BAUD, SERIAL_8N1, PIN_UART_TRADER_RX, PIN_UART_TRADER_TX);
            trader_serial->setTimeout(5);

            rpi_serial->begin(UART_PI_BAUD);
            rpi_serial->setTimeout(5);
        }   

        void handle() {
            // Read UARTS
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

            // Set the trader to disconnected if the last update extends the duration
            if ((millis() > (trader_stamp + BUGSY_TRADER_MIN_UPDATES)) && (trader_state != TraderState::DISCONNECTED)){
                trader_state = TraderState::DISCONNECTED;
                log_error("> [bugsy_core::io::handle()] Trader disconnected through timeout!");
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