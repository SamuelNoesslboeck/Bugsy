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
    Serial.setTimeout(5);

    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_BUILTIN, LOW);

    bool connected = bug_magic::remote::setup();

    if (!connected) {
        while (!bug_magic::remote::bt_serial.connected(1000)) {
            // Serial.print(".");
        }
    }

    digitalWrite(LED_BUILTIN, HIGH);
    bug_magic::remote::bt_serial.setTimeout(5);
}

void loop() {
    // Serial bridge from USB to Bluetooth
    if (Serial.available()) {
        bug_magic::remote::bt_serial.write(
            Serial.read()
        ); 
    }

    if (bug_magic::remote::bt_serial.available()) {
        Serial.write(
            bug_magic::remote::bt_serial.read()
        );
    }
}