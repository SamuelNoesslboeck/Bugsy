# include <LiquidCrystal.h>

// Local libraries
# include "libs/sylo/logging.hpp"

# define LOG_LEVEL LOG_LEVEL_TRACE

// Local headers
# include "bugsy_trader.hpp"

namespace bugsy_trader {
    namespace core {
        void test() {
            io::send_core(bugsy_core::Command::Test);
        }

        bugsy_core::Status get_status() {
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
}

void loop() {
    if (bugsy_trader::core::status != bugsy_core::Status::Running) {
        log_info("> Connecting to core ... ");

        while (bugsy_trader::core::status != bugsy_core::Status::Running) {
            delay(100);
            bugsy_trader::core::status = bugsy_trader::core::get_status();
        }

        // Send ready connection success to core
        bugsy_trader::core::trader_ready();

        log_infoln("done!");
    }

    // log_infoln(bugsy_trader::core::get_wifi_ssid());

    delay(1000);
}