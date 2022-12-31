var<push_constant> view: mat4x4<f32>;
var<push_constant> proj: mat4x4<f32>;
var<push_constant> model: mat4x4<f32>;

@vertex
fn vs_main(
@builtin(vertex_index) vi: u32,
) -> @builtin(position) vec4<f32> {
    return vec4<f32>(0.0, 0.0, 0.0, 1.0);
}
