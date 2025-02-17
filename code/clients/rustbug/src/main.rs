use core::time::Duration;
use std::io::ErrorKind;
use std::sync::{Arc, Mutex};
use std::time::Instant;

use clap::{arg, command, value_parser, Command};
use colored::{ColoredString, Colorize};

#[macro_use]
extern crate crossterm;

use crossterm::cursor;
use crossterm::event::{read, Event, KeyCode, KeyEvent, KeyEventKind, KeyEventState, KeyModifiers};
use crossterm::style::Print;
use crossterm::terminal::{disable_raw_mode, enable_raw_mode, Clear, ClearType};
use std::io::stdout;

mod bugsy;
pub use bugsy::{BugsySerial, Movement, State};

fn bool_to_colored_text(b : bool) -> ColoredString {
    if b {
        b.to_string().bright_green()
    } else {
        b.to_string().bright_red()
    }
}

#[derive(Default)]
pub struct KeyState {
    pub w : bool,
    pub a : bool,
    pub s : bool,
    pub d : bool
}

impl KeyState {
    pub fn has_none(&self) -> bool {
        !(self.w || self.a || self.s || self.d)
    }
}

fn main() {
    let mut command = command!() // requires `cargo` feature
        .arg(arg!([port] "Serial port to work with"))
        .subcommand(
            Command::new("ctrl")
                .about("Enables full control mode")
        )
        .subcommand(
            Command::new("move_test")
                .about("Performs a movement check")
                .arg(
                    arg!([time] "Time for each movement in ms")
                    .value_parser(value_parser!(u64))
                )
        )
        .subcommand(
            Command::new("ping")
                .about("Pings the robot")
                .arg(
                    arg!([interval] "Interval time")
                    .value_parser(value_parser!(u64))
                )
        )
        .subcommand(
            Command::new("status")
                .about("Prints the curret status of the bugsy")
        );


    let matches = command.get_matches_mut();

    let port = matches.get_one::<String>("port").expect("[ERROR] A valid serial interface has to be provided!");

    let mut bugsy = BugsySerial::new(port);

    if let Some(_) = matches.subcommand_matches("ctrl") {
        println!("{}", "> Enabling full control ... ".bold());

        let state_mutex = Arc::new(Mutex::new(KeyState::default()));
        let state_m = state_mutex.clone();

        #[allow(unused)]
        let thread = std::thread::spawn(move || {
            loop {
                let state = state_m.lock().unwrap();

                if state.w {
                    bugsy.send_move(&Movement::FORWARD).unwrap();
                } else if state.a {
                    bugsy.send_move(&Movement::SPIN_CCW).unwrap();
                } else if state.s {
                    bugsy.send_move(&Movement::BACKWARD).unwrap();
                } else if state.d {
                    bugsy.send_move(&Movement::SPIN_CW).unwrap();
                } else {
                    bugsy.send_move(&Movement::NONE).unwrap();
                }

                drop(state);    // Release lock
                
                // Make loop slower
                std::thread::sleep(Duration::from_millis(20));
            }
        });

        let mut stdout = stdout();
        // Going into raw mode
        enable_raw_mode().unwrap();
    
        // Clearing the screen, going to top left corner and printing welcoming message
        execute!(stdout, Clear(ClearType::All), cursor::MoveTo(0, 0), Print(r#"ctrl + q to exit, ctrl + h to print "Hello world", alt + t to print "crossterm is cool""#))
                .unwrap();
    
        // Key detection
        loop {
            // going to top left corner
            execute!(stdout, cursor::MoveTo(0, 0)).unwrap();

            // matching the key
            match read().unwrap() {
                Event::Key(KeyEvent {
                    code: KeyCode::Char('w'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Press,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().w = true; },
                Event::Key(KeyEvent {
                    code: KeyCode::Char('a'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Press,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().a = true; },
                Event::Key(KeyEvent {
                    code: KeyCode::Char('s'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Press,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().s = true; },
                Event::Key(KeyEvent {
                    code: KeyCode::Char('d'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Press,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().d = true; },

                Event::Key(KeyEvent {
                    code: KeyCode::Char('w'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Release,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().w = false; },
                Event::Key(KeyEvent {
                    code: KeyCode::Char('a'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Release,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().a = false; },
                Event::Key(KeyEvent {
                    code: KeyCode::Char('s'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Release,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().s = false; },
                Event::Key(KeyEvent {
                    code: KeyCode::Char('d'),
                    modifiers: KeyModifiers::NONE,
                    kind: KeyEventKind::Release,
                    state: KeyEventState::NONE
                }) => { state_mutex.lock().unwrap().d = false; },

                Event::Key(KeyEvent {
                    code: KeyCode::Char('c'),
                    modifiers: KeyModifiers::CONTROL,
                    kind: KeyEventKind::Press,
                    state: KeyEventState::NONE
                }) => break,

                _ => (),
            }
        }
    
        // disabling raw mode
        disable_raw_mode().unwrap();

        return;
    }

    if let Some(matches) = matches.subcommand_matches("move_test") {
        let time_ms : u64 = *matches.get_one("time").unwrap_or(&1000);
        let dur = Duration::from_millis(time_ms);

        println!("{}", "> Running movement test ... ".bold());

        println!("| > Moving {}", "forward".green());
        bugsy.movement(&Movement::FORWARD, dur).expect("[ERROR] Movement unsuccessful");

        println!("| > Moving {}", "backward".red());
        bugsy.movement(&Movement::BACKWARD, dur).expect("[ERROR] Movement unsuccessful");

        println!("| > Spinning {}", "clockwise".blue());
        bugsy.movement(&Movement::SPIN_CW, dur).expect("[ERROR] Movement unsuccessful");
        
        println!("| > Spinning {}", "counter-clockwise".blue());
        bugsy.movement(&Movement::SPIN_CCW, dur).expect("[ERROR] Movement unsuccessful");
    }

    if let Some(matches) = matches.subcommand_matches("ping") {
        let interval_ms : u64 = *matches.get_one("interval").unwrap_or(&1000);
        let dur = Duration::from_millis(interval_ms);
        let mut counter = 0;

        println!("{}", "> Pinging the Bugsy robot ... ".bold());

        loop {
            // Manual bugsy.get_state()
            bugsy.write_cmd(bugsy::Command::GetState).expect("[ERROR] Error while sending the command!");
            let inst = Instant::now();

            let state : State = unsafe {
                match bugsy.read_obj(1) {
                    Ok(state) => state,
                    Err(err) => {
                        if err.kind() == ErrorKind::TimedOut {
                            println!("| > [{}] Request timed out!", counter);
                            std::thread::sleep(dur);
                            counter += 1;
                            continue;
                        } else {
                            panic!("[ERROR] Error while reading input");
                        }
                    }
                }
            };

            let elapsed = inst.elapsed().as_millis();

            println!("| > [{}] Response state: {}, {}", counter, state, format!("{}ms", elapsed).bold());
            std::thread::sleep(dur);

            counter += 1;
        }
    }

    if let Some(_) = matches.subcommand_matches("status") {
        println!("{}", "> Bugsy - General status".bold());
        println!("| > Core status: {}", bugsy.get_state().unwrap());
        println!("| > Trader: {}", bool_to_colored_text(bugsy.is_trader_ready().unwrap()));
        println!("| > RPi: {}", bool_to_colored_text(bugsy.is_rpi_ready().unwrap()));
    }
}
