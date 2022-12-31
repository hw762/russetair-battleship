use graphics::GraphicsSystem;
use winit::{event_loop::{EventLoop, ControlFlow}, event::{WindowEvent, Event, KeyboardInput, ElementState, VirtualKeyCode}};

mod graphics;

fn main() {
    // HACK: The entire main function is a hack.
    let event_loop = EventLoop::new();
    let window = winit::window::Window::new(&event_loop).unwrap();
    window.set_resizable(false);

    let graphics = pollster::block_on(GraphicsSystem::new(&window));

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
