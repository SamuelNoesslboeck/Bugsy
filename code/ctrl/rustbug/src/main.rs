use clap::{arg, command, Command};
use colored::Colorize;

mod bugsy;
pub use bugsy::BugsySerial;

fn main() {
    let mut command = command!() // requires `cargo` feature
        .arg(arg!([port] "Serial port to work with"))
        .subcommand(
            Command::new("status")
                .about("Prints the curret status of the bugsy")
        )
        .subcommand(
            Command::new("remote")
        );

    let matches = command.get_matches_mut();

    let port = matches.get_one::<String>("port").expect("[ERROR] A valid serial interface has to be provided!");

    let mut bugsy = BugsySerial::new(port);

    if let Some(_) = matches.subcommand_matches("status") {
        println!("{}", "> Bugsy - General status".bold());
        println!("| > Core status: {}", bugsy.get_status().unwrap());
        println!("| > Trader: {}", bugsy.is_trader_ready().unwrap().to_string().blue());
        println!("| > RPi: {}", bugsy.is_rpi_ready().unwrap().to_string().blue());
    }
}
