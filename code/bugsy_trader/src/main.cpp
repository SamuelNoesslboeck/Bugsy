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

    bugsy::PrimarySensorData primary_sensor_data = bugsy::PrimarySensorData {
        /* distance_mcs_front */    0,
        /* distance_mcs_back */     0,

        /* distance_mm_front */     0,
        /* distance_mm_back */      0,

        /* accel_x */               0.0,
        /* accel_y */               0.0,
        /* accel_z */               0.0
    };

    namespace core {
        bugsy::CoreState state = bugsy::CoreState::ERROR;

        void reconnect() {
            log_info("> Connecting to core ...");

            while (!bugsy::is_operational(bugsy_trader::core::state)) {
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

        void publish_primary_sensor_data(bugsy::PrimarySensorData* data) {
            io::send_obj_core(bugsy::Command::PublishPrimarySensorData, data);
        }

        char* get_wifi_ssid() {
            io::send_cmd_core(bugsy::Command::GetWiFiSSID);
            return io::recv_obj_core<char>();
        }
    }

    namespace device {
        Adafruit_MPU6050 mpu;

        void setup() {
            if (!bugsy_trader::device::mpu.begin()) {
                log_errorln("> [setup] MPU Sensor unreachable!");
            }
        
            pinMode(PIN_SONAR_FRONT_TRIG, OUTPUT);
            pinMode(PIN_SONAR_FRONT_ECHO, INPUT);

            pinMode(PIN_SONAR_BACK_TRIG, OUTPUT);
            pinMode(PIN_SONAR_BACK_ECHO, INPUT);
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

    bugsy_trader::state = bugsy::TraderState::SETUP;

    bugsy_trader::io::setup();
    bugsy_trader::device::setup();

    log_infoln("> [setup] Done!");

    // Block until first conenction to core is built up
    bugsy_trader::state = bugsy::TraderState::CONNECTING;
    bugsy_trader::core::reconnect();
    bugsy_trader::state = bugsy::TraderState::ACTIVE;

    // Configure intervals based on timings
    state_interval.set(BUGSY_STATE_INTERVAL);
    primary_interval.set(BUGSY_PRIMARY_SENSOR_INTERVAL);
    secondary_interval.set(BUGSY_SECONDARY_SENSOR_INTERVAL);
}

void loop() {
    // Check up state when the interval has elapsed
    if (state_interval.has_elapsed()) {
        bugsy_trader::core::state = bugsy_trader::core::set_trader_state(bugsy_trader::state);

        if (!bugsy::is_operational(bugsy_trader::core::state)) {
            bugsy_trader::state = bugsy::TraderState::CONNECTING;
            bugsy_trader::core::reconnect();        // Blocks until reconnected
            bugsy_trader::state = bugsy::TraderState::ACTIVE;
        }

        state_interval.set();
    }
 
    // Check if the primary sensor data is due
    if (primary_interval.has_elapsed()) {
        primary_interval.set();     // Reset the interval

        // Fetch new primary sensor data
        // Front sensor
            digitalWrite(PIN_SONAR_FRONT_TRIG, LOW);
            delayMicroseconds(2);
            digitalWrite(PIN_SONAR_FRONT_TRIG, HIGH);
            delayMicroseconds(10);
            digitalWrite(PIN_SONAR_FRONT_TRIG, LOW);

            // About 1.5 meters range
            bugsy_trader::primary_sensor_data.distance_mcs_front = pulseIn(PIN_SONAR_FRONT_ECHO, HIGH, BUGSY_SONAR_TIMEOUT_MCS); 
            bugsy_trader::primary_sensor_data.distance_mm_front = (bugsy_trader::primary_sensor_data.distance_mcs_front * 343) / 2000;
        // 

        // Back sensor
            digitalWrite(PIN_SONAR_BACK_TRIG, LOW);
            delayMicroseconds(2);
            digitalWrite(PIN_SONAR_BACK_TRIG, HIGH);
            delayMicroseconds(10);
            digitalWrite(PIN_SONAR_BACK_TRIG, LOW);

            // About 1.5 meters range
            bugsy_trader::primary_sensor_data.distance_mcs_back = pulseIn(PIN_SONAR_BACK_ECHO, HIGH, BUGSY_SONAR_TIMEOUT_MCS); 
            bugsy_trader::primary_sensor_data.distance_mm_back = (bugsy_trader::primary_sensor_data.distance_mcs_back * 343) / 2000;
        // 

        // MPU
            sensors_event_t a, g, temp;     // Union structure

            bugsy_trader::device::mpu.getEvent(&a, &g, &temp);
            
            bugsy_trader::primary_sensor_data.accel_x = a.acceleration.x;
            bugsy_trader::primary_sensor_data.accel_y = a.acceleration.y;
            bugsy_trader::primary_sensor_data.accel_z = a.acceleration.z;
        // 

        // Print out sensor data for debugging
            log_trace("> [PrimarySensorData] {\n    sonar_front: (");
            log_trace(bugsy_trader::primary_sensor_data.distance_mcs_front);
            log_trace("mcs | ");
            log_trace(bugsy_trader::primary_sensor_data.distance_mm_front);
            log_trace("mm),\n    sonar_back: (");
            log_trace(bugsy_trader::primary_sensor_data.distance_mcs_back);
            log_trace("mcs | ");
            log_trace(bugsy_trader::primary_sensor_data.distance_mm_back);
            log_trace("mm),\n    accel: (");
            log_trace(bugsy_trader::primary_sensor_data.accel_x);
            log_trace("/");
            log_trace(bugsy_trader::primary_sensor_data.accel_y);
            log_trace("/");
            log_trace(bugsy_trader::primary_sensor_data.accel_z);
            log_trace(")\n}\n")
        // 

        bugsy_trader::core::publish_primary_sensor_data(&bugsy_trader::primary_sensor_data);
    }

    // Check if the secondary sensor data is due
    if (secondary_interval.has_elapsed()) {
        secondary_interval.set();   // Reset the interval 
    }
}