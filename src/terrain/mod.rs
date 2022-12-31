mod sector;
mod chunk;

use std::iter::once;
use std::mem::size_of;
use hecs::World;
use wgpu::*;
use wgpu::util::RenderEncoder;
pub use sector::*;
pub use chunk::*;
use crate::graphics::GraphicsSystem;

pub struct Terrain {
    shader: ShaderModule,
    model_mat: uv::Mat4x4,
}

impl Terrain {
    pub fn new(device: &Device) -> Terrain {
        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("TerrainShader"),
            source: wgpu::ShaderSource::Wgsl(include_str!("terrain.wgsl").into()),
        });
        Self { shader, model_mat: uv::Mat4x4::identity() }
    }
    pub fn layout(&self, device: &Device) -> PipelineLayout {
        let s = size_of::<uv::Mat4>() as u32;
        device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label: Some("TerrainRenderer Pipeline Layout"),
            bind_group_layouts: &[],
            push_constant_ranges: &[
                // // View matrix
                // PushConstantRange {
                //     stages: ShaderStages::VERTEX,
                //     range: 0..s,
                // },
                // // Projection matrix
                // PushConstantRange {
                //     stages: ShaderStages::VERTEX,
                //     range: s..2 * s,
                // },
                // // View matrix
                // PushConstantRange {
                //     stages: ShaderStages::VERTEX,
                //     range: 2 * s..3 * s,
                // }
            ],
        })
    }
    pub fn pipeline(&self, device: &Device, format: TextureFormat) -> RenderPipeline {
        device.create_render_pipeline(&RenderPipelineDescriptor {
            label: Some("TerrainRenderer Piipeline"),
            layout: Some(&self.layout(&device)),
            vertex: VertexState {
                module: &self.shader,
                entry_point: "vs_main",
                buffers: &[],
            },
            primitive: PrimitiveState {
                topology: wgpu::PrimitiveTopology::TriangleList, // 1.
                strip_index_format: None,
                front_face: wgpu::FrontFace::Ccw, // 2.
                cull_mode: Some(wgpu::Face::Back),
                // Setting this to anything other than Fill requires Features::NON_FILL_POLYGON_MODE
                polygon_mode: wgpu::PolygonMode::Fill,
                // Requires Features::DEPTH_CLIP_CONTROL
                unclipped_depth: false,
                // Requires Features::CONSERVATIVE_RASTERIZATION
                conservative: false,
            },
            depth_stencil: None,
            multisample: wgpu::MultisampleState {
                count: 1, // 2.
                mask: !0, // 3.
                alpha_to_coverage_enabled: false, // 4.
            },
            fragment: Some(FragmentState { // 3.
                module: &self.shader,
                entry_point: "fs_main",
                targets: &[Some(ColorTargetState { // 4.
                    format: format,
                    blend: Some(BlendState::REPLACE),
                    write_mask: ColorWrites::ALL,
                })],
            }),
            multiview: None,
        })
    }
    pub fn render(&self, device: &Device, encoder: &mut CommandEncoder, view: &TextureView, format: TextureFormat) {
        let pipeline = self.pipeline(device, format);
        let mut render_pass = encoder.begin_render_pass(&RenderPassDescriptor {
            label: Some("Render Pass"),
            color_attachments: &[
                // This is what @location(0) in the fragment shader targets
                Some(wgpu::RenderPassColorAttachment {
                    view: &view,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load: wgpu::LoadOp::Clear(
                            wgpu::Color {
                                r: 0.4,
                                g: 0.2,
                                b: 0.3,
                                a: 1.0,
                            }
                        ),
                        store: true,
                    }
                })
            ],
            depth_stencil_attachment: None,
        });

        // NEW!
        render_pass.set_pipeline(&pipeline); // 2.
        render_pass.draw(0..3, 0..1); // 3.
    }
}