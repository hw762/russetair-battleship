mod sector;
mod chunk;

use std::mem::size_of;
use hecs::World;
use wgpu::*;
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
                // View matrix
                PushConstantRange {
                    stages: ShaderStages::VERTEX,
                    range: 0..s,
                },
                // Projection matrix
                PushConstantRange {
                    stages: ShaderStages::VERTEX,
                    range: s..2 * s,
                },
                // View matrix
                PushConstantRange {
                    stages: ShaderStages::VERTEX,
                    range: 2 * s..3 * s,
                }
            ],
        })
    }
    pub fn pipeline(&self, device: &Device) -> RenderPipeline {
        device.create_render_pipeline(&RenderPipelineDescriptor {
            label: Some("TerrainRenderer Piipeline"),
            layout: Some(&self.layout(device)),
            vertex: VertexState {
                module: &self.shader,
                entry_point: "vs_main",
                buffers: &[],
            },
            primitive: Default::default(),
            depth_stencil: None,
            multisample: Default::default(),
            fragment: None,
            multiview: None,
        })
    }
}