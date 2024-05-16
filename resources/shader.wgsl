struct UniformData {
    model_matrix: mat4x4f,
    view_matrix: mat4x4f,
    projection_matrix: mat4x4f,
    camera_world_position: vec3f,
    time: f32
};

@group(0) @binding(0) var<uniform> uUniformData: UniformData;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
    @location(3) uv: vec2f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) normal: vec3f,
    @location(1) color: vec3f,
    @location(2) viewDirection: vec3f,
}

@vertex
fn vs_main (in: VertexInput, @builtin(vertex_index) i: u32 ) -> VertexOutput {
    var out: VertexOutput;
    let worldPosition = uUniformData.model_matrix * vec4f(in.position, 1.0);
    out.position = uUniformData.projection_matrix * uUniformData.view_matrix * worldPosition;
    
    out.normal = in.normal;
    out.viewDirection = in.position - uUniformData.camera_world_position;
    
	return out;
}

@fragment
fn fs_main (in: VertexOutput) -> @location(0) vec4f {
    let lightPos = vec3f(sin(uUniformData.time)*5.0, 10.0, cos(uUniformData.time)*5.0);
    
    let kh = 80.0;
    let kd = 1.0;
    let ks = 1.0;
    let ka = 0.1;

    let L = normalize(lightPos);
    let N = normalize(in.normal);
    let V = normalize(in.viewDirection);
    let H = normalize(L - V);

    let ambient = vec3f(1.0);
    let diffuse = max(0.0, dot(N, L)) * vec3f(1.0);
    let specular = max(0.0, pow(dot(N, H), kh)) * vec3f(1.0);
    let color = (kd * diffuse) + (ks * specular) + (ka * ambient);
    // color correction
    let linear_color = pow(color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
}