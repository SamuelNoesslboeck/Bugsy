# pragma once

# define BUG_MAGIC_DEVICE_NAME "bug-magic"

// External libraries
# include <BluetoothSerial.h>

// Local libraries
# include "bugsy.hpp"
# include "spellbox.hpp"

using bugsy_core::Command;

namespace bug_magic {
    namespace core {

    }

    namespace remote {
        BluetoothSerial bt_serial;

        bool start_bt();

        bool setup();

        void send_cmd(Command cmd);
    }
}