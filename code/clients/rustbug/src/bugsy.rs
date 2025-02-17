use core::time::Duration;

use colored::Colorize;
use serialport::SerialPort;

/// A standard command used by the bugsy robot, see the `bugsy.hpp` header file for more infos
#[repr(u8)]
#[derive(Clone, Copy, Debug)]
pub enum Command {
    Test = 0x00,
    GetState = 0x01,

    Move = 0x10,
    
    IsTraderReady = 0x21,
    IsRPiReady = 0x23,

    RemoteMode = 0x40
}

/// The current state of the Bugsy
#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum State {
    /// No state has been set yet
    NONE = 0x00,
    /// The controller is currently setting up
    SETUP = 0x10,
    /// The robot is in standby mode
    STANDBY = 0x20,
    /// The controller is at full activity and running
    DRIVING = 0x21,
    /// The controller has stopped due to a critical error
    ERROR = 0xF0
}

impl core::fmt::Display for State {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        // Prints out a colored version of the state
        match self {
            Self::NONE => f.write_fmt(format_args!("{}", "NONE".white())),
            Self::SETUP => f.write_fmt(format_args!("{}", "SETUP".yellow())),
            Self::STANDBY => f.write_fmt(format_args!("{}", "STANDBY".bright_blue())),
            Self::DRIVING => f.write_fmt(format_args!("{}", "RUNNING".green())),
            Self::ERROR => f.write_fmt(format_args!("{}", "ERROR".red()))
        }
    }
}

/// Current remote configuration mode of the Bugsy
#[derive(Clone, Copy, Debug)]
#[repr(u8)]
#[allow(non_camel_case_types)]
pub enum Remote {
    NONE = 0x00,

    BLUETOOTH = 0x01,

    USB = 0x02,
    TRADER = 0x04,
    RPI = 0x08,

    WIFI_UDP = 0x10,
    WIFI_TCP = 0x20,
    WIFI_MQTT = 0x40,
    WIFI_CAM = 0x80,

    ANY_WIFI = 0xF0,

    ALL = 0xFF
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
            pub fn get_state(&mut self) -> Result<State, std::io::Error> {
                self.write_cmd(Command::GetState)?;
                unsafe {
                    self.read_obj(1)
                }
            } 

            pub fn is_trader_ready(&mut self) -> Result<bool, std::io::Error> {
                self.write_cmd(Command::IsTraderReady)?;
                unsafe {
                    self.read_obj(1)
                }
            } 

            pub fn is_rpi_ready(&mut self) -> Result<bool, std::io::Error> {
                self.write_cmd(Command::IsRPiReady)?;
                unsafe {
                    self.read_obj(1)
                }
            }

            pub fn send_move(&mut self, movement : &Movement) -> Result<(), std::io::Error> {
                unsafe {
                    self.write_cmd_obj(Command::Move, movement)?;
                };
                Ok(())
            }

            pub fn remote_mode(&mut self) -> Result<Remote, std::io::Error> {
                self.write_cmd(Command::RemoteMode)?;
                unsafe {
                    self.read_obj(1)
                }
            }
        //
    }
//