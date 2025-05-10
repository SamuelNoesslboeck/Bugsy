#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")] // hide console window on Windows in release
#![allow(rustdoc::missing_crate_level_docs)] // it's an example

use core::cell::RefCell;
use std::rc::Rc;

use buglib::BugsySerial;
use eframe::egui;
use egui::{Key, ScrollArea};

mod connect;
pub use connect::*;

fn main() -> eframe::Result {
    env_logger::init();

    let serial_rc = Rc::new(RefCell::new(None));

    let connect_app_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_resizable(false)
            .with_inner_size([1000.0, 400.0]),
        ..Default::default()
    };

    let main_app_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1200.0, 800.0]),
        ..Default::default()
    };

    eframe::run_native(
        "RustBug - Connect", 
        connect_app_options, 
        Box::new(|cc| {
            egui_extras::install_image_loaders(&cc.egui_ctx);
            Ok(Box::new(RustBugConnectApp::with_serial(serial_rc.clone())))
        })
    )?; 

    // Only opens next window if successfully connected
    if let Some(serial) = serial_rc.borrow_mut().take() {
        eframe::run_native(
            "RustBug",
            main_app_options,
            Box::new(|_cc| Ok(Box::new(RustBugMainApp::with_serial(serial))))
        )?;
    }

    Ok(())
}

struct RustBugMainApp {
    console: String,
    serial : BugsySerial
}

impl RustBugMainApp {
    pub fn with_serial(serial : BugsySerial) -> Self {
        Self {
            serial,
            console: String::new()
        }
    }    
}

impl eframe::App for RustBugMainApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("RustBug");
            if ui.button("Clear").clicked() {
                self.console.clear();
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