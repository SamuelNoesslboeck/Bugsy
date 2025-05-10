use core::cell::RefCell;
use std::rc::Rc;

use buglib::BugsySerial;
use eframe::egui::{self, Vec2};

#[derive(Default)]
pub struct RustBugConnectApp {
    console: String,

    com_port : String,
    serial : Rc<RefCell<Option<BugsySerial>>>
}

impl RustBugConnectApp {
    pub fn with_serial(serial : Rc<RefCell<Option<BugsySerial>>>) -> Self {
        Self {
            serial,
            ..Default::default()
        }
    }    
}

impl eframe::App for RustBugConnectApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.vertical_centered_justified(|ui| {
                ui.add(
                    egui::Image::new(egui::include_image!("../images/bugsy_logo_text.png"))
                        .max_size(Vec2::new(800.0, 300.0))
                );
    
                ui.horizontal_centered(|ui| {
                    let com_port_label = ui.label("Com-Port: ");
                    ui.text_edit_singleline(&mut self.com_port)
                        .labelled_by(com_port_label.id);
                });
    
                if ui.button("Connect").clicked() {
                    if self.serial.borrow().is_none() {
                        match BugsySerial::connect(self.com_port.trim()) {
                            Ok(val) => { 
                                *self.serial.borrow_mut() = Some(val); 
                                ui.ctx().send_viewport_cmd(egui::ViewportCommand::Close);
                            },
                            Err(err) => {
                                self.console.push_str(err.to_string().as_str());
                            }
                        }
                    }
                }
            });
        });
    }
}