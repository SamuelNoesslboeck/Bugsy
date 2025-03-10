// Libraries
# include <bugsy/defines.hpp>
# include <sylo/logging.hpp>
# include <sylo/timing/timer.hpp>

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "bugsy_trader.hpp"

// Static fields
static Timer state_interval, primary_interval, secondary_interval;

namespace bugsy_trader {
    bugsy::TraderState state;

    namespace core {
        void reconnect() {
            log_info("> Connecting to core ...");

            while (bugsy::is_operational(bugsy_trader::core::state)) {
                delay(100);
                log_info(".");
                bugsy_trader::core::state = bugsy_trader::core::set_trader_state(bugsy_trader::state);
            }

            // Send ready connection success to core
            bugsy_trader::core::set_trader_state(bugsy_trader::state);

            log_infoln(" done!");
        }

        // Commands
        void test() {
            io::send_cmd_core(bugsy::Command::Test);
        }

        bugsy::CoreState get_state() {
            io::send_cmd_core(bugsy::Command::GetState);
            // Write to parse buffers, so `CoreState::ERROR` gets parsed when no message has been received
            io::parse_buffer[0] = (uint8_t)bugsy::CoreState::ERROR;
            return *io::recv_obj_core<bugsy::CoreState>();
        }

        bugsy::CoreState set_trader_state(bugsy::TraderState state) {
            io::send_obj_core(bugsy::Command::SetTraderState, &state);
            // Write to parse buffers, so `CoreState::ERROR` gets parsed when no message has been received
            io::parse_buffer[0] = (uint8_t)bugsy::CoreState::ERROR;
            return *io::recv_obj_core<bugsy::CoreState>();

        }

        char* get_wifi_ssid() {
            io::send_cmd_core(bugsy::Command::GetWiFiSSID);
            return io::recv_obj_core<char>();
        }
    }

    namespace io {
        HardwareSerial* core_serial = &Serial3;
        char parse_buffer [PARSE_BUFFER_SIZE];

        void setup() {
            core_serial->begin(BUGSY_UART_CORE_TO_TRADER_BAUD);
            core_serial->setTimeout(15);
        }

        void send_cmd_core(bugsy::Command cmd) {
            core_serial->write((const uint8_t*)&cmd, sizeof(bugsy::Command));
        }

        template<typename T>
        void send_obj_core(bugsy::Command cmd, T* obj) {
            core_serial->write((const uint8_t*)&cmd, sizeof(bugsy::Command));
            core_serial->write((const uint8_t*)obj, sizeof(T));
        }

        template<typename T>
        T* recv_obj_core() {
            core_serial->readBytes((uint8_t*)parse_buffer, PARSE_BUFFER_SIZE);
            return (T*)parse_buffer;
        }
    }
}

void setup() {
    // Init logging with specified baud rate
    init_logging(BUGSY_TRADER_DEBUG_BAUD);

    log_infoln("");
    log_debugln("######################");
    log_debugln("##   BUGSY-TRADER   ##");
    log_debugln("######################");
    log_debugln("|");
    log_info("> Bugsy Software Version: '");
    log_info(BUGSY_SOFTWARE_VERSION);
    log_debugln("'");
    log_debugln("|");

    bugsy_trader::io::setup();

    log_infoln("> SETUP done!");

    bugsy_trader::core::reconnect();
    state_interval.set(BUGSY_STATE_INTERVAL);
    primary_interval.set(BUGSY_PRIMARY_SENSOR_INTERVAL);
    secondary_interval.set(BUGSY_SECONDARY_SENSOR_INTERVAL);
}

void loop() {
    // Check up state
    if (state_interval.has_elapsed()) {
        bugsy_trader::core::state = bugsy_trader::core::set_trader_state(bugsy_trader::state);

        //
        if (!bugsy::is_operational(bugsy_trader::core::state)) {
            bugsy_trader::core::reconnect();
        }

        state_interval.set();
    }

    if (primary_interval.has_elapsed()) {
        primary_interval.set();
    }

    if (secondary_interval.has_elapsed()) {
        secondary_interval.set();
    }
}