// Libraries
# include <bugsy/defines.hpp>
# include <sylo/logging.hpp>
# include <sylo/timing/timer.hpp>

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "bugsy_trader.hpp"

// Static fields
static Timer status_interval;

namespace bugsy_trader {
    bugsy::TraderState state;

    namespace core {
        void reconnect() {
            log_info("> Connecting to core ...");

            while (bugsy::is_operational(bugsy_trader::core::state)) {
                delay(100);
                log_info(".");
                bugsy_trader::core::state = bugsy_trader::core::set_trader_status(bugsy_trader::state);
            }

            // Send ready connection success to core
            bugsy_trader::core::set_trader_status(bugsy_trader::state);

            log_infoln(" done!");
        }

        // Commands
        void test() {
            io::send_cmd_core(bugsy::Command::Test);
        }

        bugsy::CoreState fetch_status() {
            io::send_cmd_core(bugsy::Command::GetState);
            return *io::recv_obj_core<bugsy::CoreState>();
        }

        bugsy::CoreState set_trader_status(bugsy::TraderState state) {
            io::send_obj_core(bugsy::Command::IsTraderReady, &state);
        }

        char* get_wifi_ssid() {
            io::send_cmd_core(bugsy_core::Command::GetWiFiSSID);
            return io::recv_obj_core<char>();
        }
    }

    namespace io {
        HardwareSerial* core_serial = &Serial3;
        char parse_buffer [PARSE_BUFFER_SIZE];

        void setup() {
            core_serial->begin(BUGSY_CORE_TO_TRADER_BAUD);
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
    status_interval.set(BUGSY_TRADER_STATUS_INTERVAL);
}

void loop() {
    // log_infoln(bugsy_trader::core::get_wifi_ssid());

    delay(1000);

    if (status_interval.has_elapsed()) {
        bugsy_trader::core::state = bugsy_trader::core::fetch_status();

        if (!bugsy::is_operational(bugsy_trader::core::state)) {
            bugsy_trader::core::reconnect();
        }

        status_interval.set();
    }
}