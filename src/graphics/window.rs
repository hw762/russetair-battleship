use fltk::{prelude::*, *};
use fltk::window::DoubleWindow;

pub struct Window {
    window: DoubleWindow,
}

impl Window {
    pub fn new() -> Self {
        let mut window = window::Window::default().with_size(400, 300);
        window.end();
        window.show();
        Self {
            window,
        }
    }
    pub fn window(&self) -> &DoubleWindow {
        &self.window
    }
}