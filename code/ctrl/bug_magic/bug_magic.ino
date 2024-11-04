# include <BluetoothSerial.h>

# include "bug_magic.hpp"

namespace bug_magic {
    namespace core {
        void test() {
            remote::send_cmd(Command::Test);
        }
    }

    namespace remote {
        bool start_bt() {
            bt_serial.begin(BUG_MAGIC_DEVICE_NAME, true);
            return bt_serial.connect(bugsy_core::MAC);
        }

        bool setup() {
            return start_bt();
        }

        void send_cmd(Command cmd) {
            bt_serial.write((const uint8_t*)&cmd, sizeof(Command));
        }
    }
}

void setup() {
    Serial.begin(115200);

    Serial.print("Connecting ...");

    bool connected = bug_magic::remote::setup();

    if (!connected) {
        while (!bug_magic::remote::bt_serial.connected(1000)) {
            Serial.print(".");
        }
    }

    Serial.println(" done!");
}

void loop() {
    bug_magic::core::test();
    Serial.println("> Test command sent!");
    delay(1000);
}