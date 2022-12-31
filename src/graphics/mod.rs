use std::iter::once;

use wgpu::*;
use winit::window::Window;

pub struct GraphicsSystem {
    instance: Instance,
    surface: Surface,
    device: Device,
    queue: Queue,
    config: SurfaceConfiguration,
    size: winit::dpi::PhysicalSize<u32>,
}

impl GraphicsSystem {
    pub async fn new(window: &Window) -> Self {
        let size = window.inner_size();
        let instance = Instance::new(Backends::all());
        let surface = unsafe { instance.create_surface(window) };
        let adapter = instance
            .request_adapter(&RequestAdapterOptions {
                power_preference: PowerPreference::HighPerformance,
                force_fallback_adapter: false,
                compatible_surface: Some(&surface),
            })
            .await
            .expect("Failed to find an appropriate adapter");
        let (device, queue) = adapter
            .request_device(
                &DeviceDescriptor {
                    label: Some("Device"),
                    features: Features::PUSH_CONSTANTS,
                    limits: Limits::default(),
                },
                None,
            )
            .await
            .expect("Failed to create device");
        let config = SurfaceConfiguration {
            usage: TextureUsages::RENDER_ATTACHMENT,
            format: surface.get_supported_formats(&adapter)[0],
            width: size.width,
            height: size.height,
            present_mode: PresentMode::Fifo,
            alpha_mode: CompositeAlphaMode::Auto,
        };
        surface.configure(&device, &config);
        Self {
            instance,
            surface,
            device,
            queue,
            config,
            size,
        }
    }
    pub fn device(&self) -> &Device {
        &self.device
    }
    pub fn encoder(&self, label: Option<&str>) -> CommandEncoder {
        self.device.create_command_encoder(&CommandEncoderDescriptor {
            label
        })
    }
    pub fn clear(&mut self) {
        let output = self
            .surface
            .get_current_texture()
            .expect("Failed to get surface texture");
        let view = output
            .texture
            .create_view(&TextureViewDescriptor::default());
        let mut encoder = self.encoder(Some("ClearEncoder"));
        encoder.begin_render_pass(&RenderPassDescriptor {
            label: Some("ClearRenderPass"),
            color_attachments: &[Some(RenderPassColorAttachment {
                view: &view,
                resolve_target: None,
                ops: Operations {
                    load: LoadOp::Clear(Color {
                        r: 0.1,
                        g: 0.2,
                        b: 0.3,
                        a: 1.0,
                    }),
                    store: true,
                },
            })],
            depth_stencil_attachment: None,
        });

        self.queue.submit(once(encoder.finish()));
        output.present();
    }
}

pub struct Camera {
    transform: uv::Mat4,
}

impl Camera {
    pub fn new() -> Self {
        // HACK Move to (0, 0, -1)
        let transform = uv::Mat4::from_translation(uv::Vec3::new(0., 0., -1.0));
        Self {
            transform
        }
    }
}