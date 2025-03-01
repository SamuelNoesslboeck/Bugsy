// #########################
// #    BUGSY - DEFINES    #
// #########################
//
// General defines that are relevant for most if not all parts of the software

# pragma once

/// @brief Current version of the bugsy software
# define BUGSY_SOFTWARE_VERSION "0.2.0/2025/02/17"

// Remotes
/// @brief Current device name 
# define BUGSY_DEVICE_NAME "bugsy"

// Baud rates
/// @brief Baud rate between the core and the trader
# define BUGSY_UART_CORE_TO_TRADER_BAUD 250000

// Intervals
# define BUGSY_STATE_INTERVAL 1000

/// @brief Minimum time between *any form of messages* between the trader and the core until the core issues the trader as disconnected
# define BUGSY_TRADER_MIN_UPDATES 1000

# define BUGSY_PRIMARY_SENSOR_INTERVAL 500

# define BUGSY_SECONDARY_SENSOR_INTERVAL 5000