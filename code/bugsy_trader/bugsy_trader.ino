# include <LiquidCrystal.h>

// Local libraries
# include "sylo/logging.hpp"
# include "sylo/timing/timer.hpp"

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "bugsy_trader.hpp"

// Static fields
static Timer status_interval;

namespace bugsy_trader {
    namespace core {
        void reconnect() {
            log_info("> Connecting to core ...");

            while (bugsy_trader::core::status != bugsy_core::Status::Running) {
                delay(100);
                log_info(".");
                bugsy_trader::core::status = bugsy_trader::core::fetch_status();
            }

            // Send ready connection success to core
            bugsy_trader::core::trader_ready();

            log_infoln(" done!");
        }

        // Commands
        void test() {
            io::send_core(bugsy_core::Command::Test);
        }

        bugsy_core::Status fetch_status() {
            io::send_core(bugsy_core::Command::Status);
            return *io::recv_core<bugsy_core::Status>();
        }

        void trader_ready() {
            io::send_core(bugsy_core::Command::TraderReady);
        }

        char* get_wifi_ssid() {
            io::send_core(bugsy_core::Command::GetWiFiSSID);
            return io::recv_core<char>();
        }
    }

    namespace io {
        void setup() {
            core_serial->begin(BUGSY_CORE_TO_TRADER_BAUD);
            core_serial->setTimeout(15);
        }

        void send_core(bugsy_core::Command cmd) {
            core_serial->write((const uint8_t*)&cmd, sizeof(bugsy_core::Command));
        }

        template<typename T>
        T* recv_core() {
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

    log_infoln("> Setup done!");

    bugsy_trader::core::reconnect();
    status_interval.set(BUGSY_TRADER_STATUS_INTERVAL);
}

void loop() {
    // log_infoln(bugsy_trader::core::get_wifi_ssid());

    delay(1000);

    if (status_interval.has_elapsed()) {
        bugsy_trader::core::status = bugsy_trader::core::fetch_status();

        if (bugsy_trader::core::status != bugsy_core::Status::Running) {
            bugsy_trader::core::reconnect();
        }

        status_interval.set();
    }
}