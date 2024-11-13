use core::time::Duration;
use colored::Colorize;
use serialport::SerialPort;

#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum Command {
    Test = 0x00,
    GetStatus = 0x01,

    
    IsTraderReady = 0x21,
    IsRPiReady = 0x23,

    RemoteMode = 0x40
}

#[derive(Clone, Copy, Debug)]
#[repr(u8)]
pub enum Status {
    SETUP = 0x00,
    RUNNING = 0x01
}

impl core::fmt::Display for Status {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::SETUP => f.write_fmt(format_args!("{}", "SETUP".yellow())),
            Self::RUNNING => f.write_fmt(format_args!("{}", "RUNNING".green()))
        }
    }
}

#[derive(Clone, Copy, Debug)]
#[repr(u8)]
#[allow(non_camel_case_types)]
pub enum RemoteMode {
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

    pub fn write_cmd(&mut self, cmd : Command) -> Result<usize, std::io::Error> {
        self.tx_buffer[0] = cmd as u8;
        self.port.write(&self.tx_buffer[0 .. 1])
    }

    pub unsafe fn read_obj<T>(&mut self, size : usize) -> Result<T, std::io::Error> {
        self.port.read_exact(&mut self.rx_buffer[0 .. size]).unwrap();
        Ok(core::mem::transmute_copy(&mut self.rx_buffer))
    }

    // Commands
        pub fn get_status(&mut self) -> Result<Status, std::io::Error> {
            self.write_cmd(Command::GetStatus)?;
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

        pub fn remote_mode(&mut self) -> Result<RemoteMode, std::io::Error> {
            self.write_cmd(Command::RemoteMode)?;
            unsafe {
                self.read_obj(1)
            }
        }
    //
}