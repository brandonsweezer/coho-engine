struct UniformData {
    model_matrix: mat4x4f,
    view_matrix: mat4x4f,
    projection_matrix: mat4x4f,
    camera_world_position: vec3f,
    time: f32
};

struct ModelData {
    transform: mat4x4f,
}

@group(0) @binding(0) var<uniform> uUniformData: UniformData;
@group(0) @binding(1) var albedo_texture: texture_2d<f32>;
@group(0) @binding(2) var normal_texture: texture_2d<f32>;
@group(0) @binding(3) var texture_sampler: sampler;
@group(0) @binding(4) var<storage, read> modelBuffer: array<ModelData>;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
    @location(3) tangent: vec3f,
    @location(4) bitangent: vec3f,
    @location(5) uv: vec2f,
    @location(6) modelId: u32,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) normal: vec3f,
    @location(1) color: vec3f,
    @location(2) viewDirection: vec3f,
    @location(3) tangent: vec3f,
    @location(4) bitangent: vec3f,
    @location(5) uv: vec2f,
}

@vertex
fn vs_main (in: VertexInput, @builtin(vertex_index) i: u32, @builtin(instance_index) instance_id: u32 ) -> VertexOutput {
    var out: VertexOutput;
    let model_matrix = modelBuffer[instance_id].transform;
    let worldPosition = model_matrix * vec4f(in.position, 1.0);
    out.position = uUniformData.projection_matrix * uUniformData.view_matrix * worldPosition;
    
    out.tangent = (uUniformData.model_matrix * vec4f(in.tangent, 0.0)).xyz;
	out.bitangent = (uUniformData.model_matrix * vec4f(in.bitangent, 0.0)).xyz;
    out.normal = (uUniformData.model_matrix * vec4f(in.normal, 0.0)).xyz;
    out.viewDirection = uUniformData.camera_world_position - worldPosition.xyz;
    out.uv = in.uv;
    
	return out;
}

@fragment
fn fs_main (in: VertexOutput) -> @location(0) vec4f {
    let normal_sample = textureSample(normal_texture, texture_sampler, in.uv).rgb;
    
    let localToWorld = mat3x3f(
        normalize(in.tangent),
        normalize(in.bitangent),
        normalize(in.normal)
    );
    
    let worldN = localToWorld * (2.0*normal_sample - 1.0);
    let N = normalize(mix(in.normal, worldN, 1.0));

    let albedo = textureSample(albedo_texture, texture_sampler, in.uv).rgb;

    var lightPositions = array(
        vec3f(sin(uUniformData.time)*5.0, 2.0, cos(uUniformData.time)*5.0),
        vec3f(cos(uUniformData.time)*5.0, -2.0, sin(uUniformData.time)*5.0)
    );
    let V = normalize(in.viewDirection);


    let kh = 80.0;
    let kd = 1.0;
    let ks = 0.5;
    let ka = 0.0;

    var color = vec3f(0.0);
    for (var i: i32 = 0; i < 2; i++) {
        var L = normalize(lightPositions[i].xyz);
        let H = normalize(L + V);

        let ambient = albedo;
        let diffuse = max(0.0, dot(N, L)) * vec3f(1.0) * albedo;
        let specular = max(0.0, pow(dot(N, H), kh))  * vec3f(1.0);
        color += (kd * diffuse) + (ks * specular) + (ka * ambient);
    }

    // color correction
    let linear_color = pow(color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
}