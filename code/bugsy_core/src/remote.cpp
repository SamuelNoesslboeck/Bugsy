# include "remote.hpp"

# include <bugsy/core.hpp>
# include <bugsy/defines.hpp>

// Local headers
# include "config.hpp"
# include "io.hpp"

using bugsy::Remote;

namespace bugsy_core {
    namespace remote {
        // Bluetooth
            BluetoothSerial bt_serial;

            bool bt_active = false;

            bool has_bt(Remote _remotes) {
                return (bool)(((uint8_t)_remotes) & ((uint8_t)Remote::BLUETOOTH));
            }

            void start_bt() {
                bt_serial.begin(BUGSY_DEVICE_NAME);
                bt_serial.setTimeout(5);

                bt_active = true;
            }

            void stop_bt() {
                bt_serial.end();

                bt_active = false;
            }
        // 

        // WiFi
            bool wifi_active = false;

            bool has_wifi(Remote _remotes) {
                return (bool)(((uint8_t)_remotes) & ((uint8_t)Remote::ANY_WIFI));
            }

            bool is_wifi_data_set() {
                return (bool)(config::wifi_ssid[0]);
            }

            CoreError start_wifi() {
                // Send msg to pi
            }   

            void stop_wifi() {
                // Send msg to pi
            }
        // 

        void configure(Remote mode) {
            // Calculate differences between currently active modes
                // Find out where differences are between the current mode and the requested one
                Remote action_req = (Remote)((uint8_t)mode ^ (uint8_t)remotes);
                // Services to turn off
                Remote turn_off = (Remote)((uint8_t)action_req & (uint8_t)remotes);
                // Services to turn on
                Remote turn_on = (Remote)((uint8_t)action_req & (uint8_t)mode);
            //

            // Activating
            if (has_bt(turn_on)) {
                log_info("| > Starting bluetooth ... ");
                start_bt();
                log_infoln("done!");

                log_debug("| | > Device name: '");
                log_debug(BUGSY_DEVICE_NAME);
                log_debugln("'");
            }

            // Deactivating
            if (has_bt(turn_off)) {
                log_info("| > Stopping bluetooth ... ");
                stop_bt();
                log_infoln("done!");
            }

            remotes = mode;
        }

        void stop_all() {
            stop_bt();
            stop_wifi();

            remotes = Remote::NONE;
        }

        void handle() {
            if (has_bt()) {
                if (bt_serial.available()) {
                    io::parse_cmd(
                        Remote::BLUETOOTH,
                        io::parse_buffer,
                        bt_serial.readBytes(io::parse_buffer, PARSE_BUFFER_SIZE)
                    );
                }
            }
        }
    }
}