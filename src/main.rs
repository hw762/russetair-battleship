mod controls;
mod scene;

use controls::Controls;
use scene::Scene;

use iced_wgpu::{wgpu, Backend, Renderer, Settings, Viewport};
use iced_winit::{
    conversion, futures, program, renderer, winit, Clipboard, Color, Debug,
    Size,
};

use winit::{
    dpi::PhysicalPosition,
    event::{Event, ModifiersState, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
};

pub fn main() {
    env_logger::init();

    // Initialize winit
    let event_loop = EventLoop::new();

    let window = winit::window::Window::new(&event_loop).unwrap();

    let physical_size = window.inner_size();
    let mut viewport = Viewport::with_physical_size(
        Size::new(physical_size.width, physical_size.height),
        window.scale_factor(),
    );
    let mut cursor_position = PhysicalPosition::new(-1.0, -1.0);
    let mut modifiers = ModifiersState::default();
    let mut clipboard = Clipboard::connect(&window);

    // Initialize wgpu
    let default_backend = wgpu::Backends::PRIMARY;

    let backend =
        wgpu::util::backend_bits_from_env().unwrap_or(default_backend);

    let instance = wgpu::Instance::new(backend);
    let surface = unsafe { instance.create_surface(&window) };

    let (format, (device, queue)) = futures::executor::block_on(async {
        let adapter = wgpu::util::initialize_adapter_from_env_or_default(
            &instance,
            backend,
            Some(&surface),
        )
            .await
            .expect("No suitable GPU adapters found on the system!");

        let adapter_features = adapter.features();

        let needed_limits = wgpu::Limits::default();

        (
            surface
                .get_supported_formats(&adapter)
                .first()
                .copied()
                .expect("Get preferred format"),
            adapter
                .request_device(
                    &wgpu::DeviceDescriptor {
                        label: None,
                        features: adapter_features & wgpu::Features::default(),
                        limits: needed_limits,
                    },
                    None,
                )
                .await
                .expect("Request device"),
        )
    });

    surface.configure(
        &device,
        &wgpu::SurfaceConfiguration {
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            format,
            width: physical_size.width,
            height: physical_size.height,
            present_mode: wgpu::PresentMode::AutoVsync,
            alpha_mode: wgpu::CompositeAlphaMode::Auto,
        },
    );

    let mut resized = false;

    // Initialize staging belt
    let mut staging_belt = wgpu::util::StagingBelt::new(5 * 1024);

    // Initialize scene and GUI controls
    let scene = Scene::new(&device, format);
    let controls = Controls::new();

    // Initialize iced
    let mut debug = Debug::new();
    let mut renderer =
        Renderer::new(Backend::new(&device, Settings::default(), format));

    let mut state = program::State::new(
        controls,
        viewport.logical_size(),
        &mut renderer,
        &mut debug,
    );

    // Run event loop
    event_loop.run(move |event, _, control_flow| {
        // You should change this if you want to render continuosly
        *control_flow = ControlFlow::Wait;

        match event {
            Event::WindowEvent { event, .. } => {
                match event {
                    WindowEvent::CursorMoved { position, .. } => {
                        cursor_position = position;
                    }
                    WindowEvent::ModifiersChanged(new_modifiers) => {
                        modifiers = new_modifiers;
                    }
                    WindowEvent::Resized(_) => {
                        resized = true;
                    }
                    WindowEvent::CloseRequested => {
                        *control_flow = ControlFlow::Exit;
                    }
                    _ => {}
                }

                // Map window event to iced event
                if let Some(event) = iced_winit::conversion::window_event(
                    &event,
                    window.scale_factor(),
                    modifiers,
                ) {
                    state.queue_event(event);
                }
            }
            Event::MainEventsCleared => {
                // If there are events pending
                if !state.is_queue_empty() {
                    // We update iced
                    let _ = state.update(
                        viewport.logical_size(),
                        conversion::cursor_position(
                            cursor_position,
                            viewport.scale_factor(),
                        ),
                        &mut renderer,
                        &iced_wgpu::Theme::Dark,
                        &renderer::Style { text_color: Color::WHITE },
                        &mut clipboard,
                        &mut debug,
                    );

                    // and request a redraw
                    window.request_redraw();
                }
            }
            Event::RedrawRequested(_) => {
                if resized {
                    let size = window.inner_size();

                    viewport = Viewport::with_physical_size(
                        Size::new(size.width, size.height),
                        window.scale_factor(),
                    );

                    surface.configure(
                        &device,
                        &wgpu::SurfaceConfiguration {
                            format,
                            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
                            width: size.width,
                            height: size.height,
                            present_mode: wgpu::PresentMode::AutoVsync,
                            alpha_mode: wgpu::CompositeAlphaMode::Auto,
                        },
                    );

                    resized = false;
                }

                match surface.get_current_texture() {
                    Ok(frame) => {
                        let mut encoder = device.create_command_encoder(
                            &wgpu::CommandEncoderDescriptor { label: None },
                        );

                        let program = state.program();

                        let view = frame.texture.create_view(&wgpu::TextureViewDescriptor::default());

                        {
                            // We clear the frame
                            let mut render_pass = scene.clear(
                                &view,
                                &mut encoder,
                                program.background_color(),
                            );

                            // Draw the scene
                            scene.draw(&mut render_pass);
                        }

                        // And then iced on top
                        renderer.with_primitives(|backend, primitive| {
                            backend.present(
                                &device,
                                &mut staging_belt,
                                &mut encoder,
                                &view,
                                primitive,
                                &viewport,
                                &debug.overlay(),
                            );
                        });

                        // Then we submit the work
                        staging_belt.finish();
                        queue.submit(Some(encoder.finish()));
                        frame.present();

                        // Update the mouse cursor
                        window.set_cursor_icon(
                            iced_winit::conversion::mouse_interaction(
                                state.mouse_interaction(),
                            ),
                        );

                        // And recall staging buffers
                        staging_belt.recall();
                    }
                    Err(error) => match error {
                        wgpu::SurfaceError::OutOfMemory => {
                            panic!("Swapchain error: {}. Rendering cannot continue.", error)
                        }
                        _ => {
                            // Try rendering again next frame.
                            window.request_redraw();
                        }
                    },
                }
            }
            _ => {}
        }
    })
}