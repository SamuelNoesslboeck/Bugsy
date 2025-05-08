#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(rustdoc::missing_crate_level_docs)] // it's an example

use buglib::BugsySerial;
use eframe::egui;
use egui::{Key, ScrollArea};

fn main() -> eframe::Result {
    env_logger::init();

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([1200.0, 800.0]),
        ..Default::default()
    };

    eframe::run_native(
        "Keyboard events",
        options,
        Box::new(|_cc| Ok(Box::<RustBugApp>::default())),
    )
}

#[derive(Default)]
struct RustBugApp {
    console: String,

    com_port : String,
    serial : Option<BugsySerial>
}

impl eframe::App for RustBugApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("RustBug");
            ui.horizontal(|ui| {
                let com_port_label = ui.label("Com-Port: ");
                ui.text_edit_singleline(&mut self.com_port)
                    .labelled_by(com_port_label.id);
            });

            if ui.button("Clear").clicked() {
                self.console.clear();
            }

            if ui.button("Connect").clicked() {
                if self.serial.is_none() {
                    match BugsySerial::connect(self.com_port.trim()) {
                        Ok(val) => { self.serial = Some(val) },
                        Err(err) => {
                            self.console.push_str(err.to_string().as_str())
                        }
                    }
                }
            }

            ScrollArea::vertical()
                .auto_shrink(false)
                .stick_to_bottom(true)
                .show(ui, |ui| {
                    ui.label(&self.console);
                });

            if ctx.input(|i| i.key_pressed(Key::A)) {
                self.console.push_str("\nPressed");
            }
            if ctx.input(|i| i.key_down(Key::A)) {
                self.console.push_str("\nHeld");
                ui.ctx().request_repaint(); // make sure we note the holding.
            }
            if ctx.input(|i| i.key_released(Key::A)) {
                self.console.push_str("\nReleased");
            }
        });
    }
}