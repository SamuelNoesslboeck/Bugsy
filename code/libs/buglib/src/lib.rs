#![crate_name = "buglib"]
#![doc = include_str!("../README.md")]

use core::time::Duration;

use colored::Colorize;
use serialport::SerialPort;

/// A standard command used by the bugsy robot, see the `bugsy.hpp` header file for more infos
#[repr(u8)]
#[derive(Clone, Copy, Debug)]
pub enum Command {
    /// Test command for debug purposes
    /// @return The additional bytes given
    Test = 0x00,

    /// State command mainly for internal communication
    /// @return `0x00` - The current `State` (see `bugsy_core::State`)
    GetState = 0x01,

    /// Issue a new movement
    /// @param `0x00-0x03` 4 byte `Movement` struct, will be parsed and applied directly, every sequence of bytes is valid!
    Move = 0x10,
    
    /// Sets the current movement mode, changing fundamentally how the Bugsy behaves
    SetMoveMode = 0x11,
    
    GetMoveMode = 0x12,

    /// Returns the current movement configuration, defining acceleration etc.
    /// @return `0x00`-sizeof(MoveConfig): The current movement configuration
    GetMoveConfig = 0x13,
    /// Sets the current movement configuration
    SetMoveConfig = 0x14,


    /// Internal command to set the stored state of the trader that will be communicated to external devices  
    /// The function also returns the current state of the Bugsy robot
    /// @param `0x00` The `bugsy_trader::State` to be stored in the core
    /// @return `0x00` The current `bugsy_core::State` of the core
    SetTraderState = 0x20,
    /// Get the (currently in the core registered) state of the trader MCU
    /// @return `0x00` The `bugsy_trader::State` value
    GetTraderState = 0x21,

    /// Internal command for publishing important (primary) sensor data to the core MCU
    /// @param `0x00-sizeof(bugsy_trader::PrimarySensorData)` the data to store
    PublishPrimarySensorData = 0x22,
    /// Returns the primary sensor data stored in the core
    /// @return `bugsy_trader::PrimarySensorData`
    GetPrimarySensorData = 0x23,
    /// Internal command for publishing less important (secondary) sensor data to the core MCU
    PublishSecondarySensorData = 0x24,
    GetSecondarySensorData = 0x25,

    /// Internal command to signal that the RPi is ready
    SetRPiReady = 0x28,
    /// Get whether the raspberry pi is ready or not
    IsRPiReady = 0x29,

    /// Returns the current remote configuration
    /// @return `0x00` - The current remote mode
    Remotes = 0x40,
    /// Reconfigures the remote settings made
    /// @param 0x00 The new `Remotes`
    RemoteConfigure = 0x41,

    /// Safe the configuration to the EEPROM
    SaveConfig = 0x80,

    /// Get the current SSID for the WiFi connection
    /// @return The WiFi SSID as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
    GetWiFiSSID = 0xA0, 
    /// Set the current SSID for the WiFi connection
    /// @param 0x00-? The WiFi SSID as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
    SetWiFiSSID = 0xA1,
    
    /// Get the current password for the WiFi connection
    /// @return The WiFi password as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
    GetWiFiPwd = 0xA2,
    /// Set the current password for the WiFi connection
    /// @param 0x00-? The WiFi password as null terminated string (for max len see `WIFI_BUFFER_SIZE`)
    SetWiFiPwd = 0xA3
}

/// The current state of the Bugsy
#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum CoreState {
    /// No state has been set yet
    NONE = 0x00,

    /// The controller is currently setting up
    SETUP = 0x10,
    /// The robot is in standby mode, saving power
    STANDBY = 0x11,

    /// The robot is active and ready to perform movements / protocols
    ACTIVE = 0x20,
    /// The controller is at full activity and running
    DRIVING = 0x21,

    /// The controller has stopped due to a critical error
    ERROR = 0x80
}

impl core::fmt::Display for CoreState {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        // Prints out a colored version of the state
        match self {
            Self::NONE => f.write_fmt(format_args!("{}", "NONE".white())),
            Self::SETUP => f.write_fmt(format_args!("{}", "SETUP".yellow())),
            Self::STANDBY => f.write_fmt(format_args!("{}", "STANDBY".bright_blue())),
            Self::ACTIVE => f.write_fmt(format_args!("{}", "ACTIVE".green())),
            Self::DRIVING => f.write_fmt(format_args!("{}", "DRIVING".blue())),
            Self::ERROR => f.write_fmt(format_args!("{}", "ERROR".red()))
        }
    }
}

/// Current remote configuration mode of the Bugsy
#[derive(Clone, Copy, Debug)]
#[repr(u8)]
#[allow(non_camel_case_types)]
pub enum Remote {
    /// Errorful `None` address, indicates that something went wrong
    NONE = 0x00,

    /// Bluetooth, the main source of remote configuration
    BLUETOOTH = 0x01,

    /// Lora, as a fast, reliable high-range communication method
    LORA = 0x02,

    // Local
    /// Direct UART connection via the USB port
    USB = 0x04,
    /// Direct UART connection to the Trader MCU
    TRADER = 0x08,
    /// Direct UART connection to the RPi 
    RPI = 0x10,

    // Wifi
    /// Wifi data, transfered by a TCP socket
    WIFI_TCP = 0x20,
    /// Wifi data, transfered using MQTT  
    WIFI_MQTT = 0x40,

    /// Any WiFi source (all when sending)
    ANY_WIFI = 0x60,

    /// Communication with the Mod-slot
    MOD = 0x80
}

#[derive(Clone, Debug)]
pub struct PrimarySensorData {
    pub distance_mcs_front : u32,
    pub distance_mcs_back : u32,

    pub distance_mm_front : u32,
    pub distance_mm_back : u32,

    pub accel_x : f32,
    pub accel_y : f32,
    pub accel_z : f32,
}

#[derive(Copy, Clone, Debug)]
pub struct Movement {
    pub chain_left_dir : bool,
    pub chain_right_dir : bool,
    pub chain_left_duty : u8,
    pub chain_right_duty : u8
}

impl Movement {
    pub const NONE : Self = Self {
        chain_left_dir: true,
        chain_right_dir: true,
        chain_left_duty: 0,
        chain_right_duty: 0
    }; 

    pub const FORWARD : Self = Self {
        chain_left_dir: true,
        chain_right_dir: true,
        chain_left_duty: u8::MAX,
        chain_right_duty: u8::MAX
    };  

    pub const BACKWARD : Self = Self {
        chain_left_dir: false,
        chain_right_dir: false,
        chain_left_duty: u8::MAX,
        chain_right_duty: u8::MAX
    };

    pub const SPIN_CW : Self = Self {
        chain_left_dir: true,
        chain_right_dir: false,
        chain_left_duty: u8::MAX,
        chain_right_duty: u8::MAX
    };

    pub const SPIN_CCW : Self = Self {
        chain_left_dir: false,
        chain_right_dir: true,
        chain_left_duty: u8::MAX,
        chain_right_duty: u8::MAX
    };    
}


// ######################
// #    BUGSY-SERIAL    #
// ######################
    pub struct BugsySerial {
        pub port : Box<dyn SerialPort>,
        
        tx_buffer : [u8; 32],
        rx_buffer : [u8; 64]
    }

    impl BugsySerial {
        pub fn new<'a>(path : impl Into<std::borrow::Cow<'a, str>>) -> Self {
            Self {
                port: serialport::new(path, 115_200)
                    .timeout(Duration::from_millis(1000))
                    .open().expect("Failed to open port"),

                tx_buffer: [0; 32],
                rx_buffer: [0; 64]
            }
        }

        // USB I/O
            pub unsafe fn read_obj<T>(&mut self, size : usize) -> Result<T, std::io::Error> {
                self.port.read_exact(&mut self.rx_buffer[0 .. size])?;
                Ok(core::mem::transmute_copy(&mut self.rx_buffer))
            }

            pub fn write_cmd(&mut self, cmd : Command) -> Result<usize, std::io::Error> {
                self.tx_buffer[0] = cmd as u8;
                self.port.write(&self.tx_buffer[0 .. 1])
            }

            pub unsafe fn write_cmd_obj<T>(&mut self, cmd : Command, obj : &T) -> Result<usize, std::io::Error> {
                self.tx_buffer[0] = cmd as u8;

                let size = core::mem::size_of::<T>();

                core::ptr::copy_nonoverlapping(
                    obj as *const T as *const u8, 
                    &mut self.tx_buffer[1] as *mut u8, 
                    size
                );

                self.port.write(&self.tx_buffer[0 .. (size + 1)])
            }
        //

        // Helper 
            pub fn movement(&mut self, movement : &Movement, mut duration : Duration) -> Result<(), std::io::Error> {
                let interval = Duration::from_millis(50);

                loop {
                    self.send_move(movement)?;

                    if duration > interval {
                        std::thread::sleep(interval);
                        duration -= interval;
                    } else {
                        std::thread::sleep(duration);
                        break;
                    }
                }

                self.send_move(&Movement::NONE)
            }
        // 

        // Commands
            pub fn get_state(&mut self) -> Result<CoreState, std::io::Error> {
                self.write_cmd(Command::GetState)?;
                unsafe {
                    self.read_obj(1)
                }
            } 

            /// TODO: Rework to State
            pub fn is_trader_ready(&mut self) -> Result<bool, std::io::Error> {
                // self.write_cmd(Command::IsTraderReady)?;
                // unsafe {
                //     self.read_obj(1)
                // }
                todo!()
            } 

            pub fn is_rpi_ready(&mut self) -> Result<bool, std::io::Error> {
                self.write_cmd(Command::IsRPiReady)?;
                unsafe {
                    self.read_obj(core::mem::size_of::<bool>())
                }
            }

            pub fn send_move(&mut self, movement : &Movement) -> Result<(), std::io::Error> {
                unsafe {
                    self.write_cmd_obj(Command::Move, movement)?;
                };
                Ok(())
            }

            pub fn remote_mode(&mut self) -> Result<Remote, std::io::Error> {
                self.write_cmd(Command::Remotes)?;
                unsafe {
                    self.read_obj(core::mem::size_of::<Remote>())
                }
            }

            pub fn get_primary_sensor_data(&mut self) -> Result<PrimarySensorData, std::io::Error> {
                self.write_cmd(Command::GetPrimarySensorData)?;
                unsafe {
                    self.read_obj(core::mem::size_of::<PrimarySensorData>())
                }
            }
        //
    }
//