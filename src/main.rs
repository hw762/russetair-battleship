use hecs::*;
use terrain::*;

use graphics::GraphicsSystem;
use winit::{event_loop::{EventLoop, ControlFlow}, event::{WindowEvent, Event, KeyboardInput, ElementState, VirtualKeyCode}};
use crate::graphics::Camera;

mod graphics;
mod terrain;

fn main() {
    let mut world = World::new();
    let _ = world.spawn(("Hello", 0));
    // HACK: The entire main function is a hack.
    let event_loop = EventLoop::new();
    let window = winit::window::Window::new(&event_loop).unwrap();
    window.set_resizable(false);

    let mut graphics = pollster::block_on(GraphicsSystem::new(&window));
    graphics.clear();
    let terrain = Terrain::new(graphics.device());

    let camera = Camera::new();
    let proj = uv::projection::orthographic_wgpu_dx(0., 800., 0., 600., 0.1, 100.);

    event_loop.run(move |event, _, control_flow| {
        match event {
            Event::WindowEvent {
                ref event,
                window_id,
            } if window_id == window.id() => 
                match event {
                    WindowEvent::CloseRequested
                    | WindowEvent::KeyboardInput {
                        input:
                            KeyboardInput {
                                state: ElementState::Pressed,
                                virtual_keycode: Some(VirtualKeyCode::Escape),
                                ..
                            },
                        ..
                    } => *control_flow = ControlFlow::Exit,
                    _ => {}
                }
            
            _ => {}
        }
    });

}
