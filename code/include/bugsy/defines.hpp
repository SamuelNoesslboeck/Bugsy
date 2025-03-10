// #########################
// #    BUGSY - DEFINES    #
// #########################
//
// General defines that are relevant for most if not all parts of the software

# pragma once

/// Current version of the bugsy software
# define BUGSY_SOFTWARE_VERSION "0.2.0/2025/02/17"

/* REMOTES */
/// Current device name 
# define BUGSY_DEVICE_NAME "bugsy"

/* BAUD RATES */
/// Baud rate between the core and the trader
# define BUGSY_UART_CORE_TO_TRADER_BAUD 250000

/* INTERVALS */
# define BUGSY_STATE_INTERVAL 1000

/// Minimum time between *any form of messages* between the trader and the core until the core issues the trader as disconnected
# define BUGSY_TRADER_MIN_UPDATES 1000
/// Time interval between measurements of the primary sensor data
# define BUGSY_PRIMARY_SENSOR_INTERVAL 500
/// Time interval between measurements of the secondary sensor data
# define BUGSY_SECONDARY_SENSOR_INTERVAL 5000

/* SIZES */
/// The buffer size for WiFi credentials (SSID and Password each)
# define BUGSY_WIFI_CRED_BUFFER_SIZE 32