struct UniformData {
    model_matrix: mat4x4f,
    view_matrix: mat4x4f,
    projection_matrix: mat4x4f,
    camera_world_position: vec3f,
    time: f32
};

struct ModelData {
    transform: mat4x4f,
    textureIndex: u32,
}

@group(0) @binding(0) var<uniform> uUniformData: UniformData;
@group(0) @binding(1) var albedo_texture: texture_2d<f32>;
@group(0) @binding(2) var normal_texture: texture_2d<f32>;
@group(0) @binding(3) var texture_sampler: sampler;
@group(0) @binding(4) var<storage, read> modelBuffer: array<ModelData>;
@group(0) @binding(5) var environment_texture: texture_2d<f32>;
@group(0) @binding(6) var radiance_texture: texture_2d<f32>;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
    @location(3) tangent: vec3f,
    @location(4) bitangent: vec3f,
    @location(5) uv: vec2f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) normal: vec3f,
    @location(1) color: vec3f,
    @location(2) viewDirection: vec3f,
    @location(3) tangent: vec3f,
    @location(4) bitangent: vec3f,
    @location(5) uv: vec2f,
    @location(6) instance_id: u32
}

@vertex
fn vs_main (in: VertexInput, @builtin(vertex_index) i: u32, @builtin(instance_index) instance_id: u32 ) -> VertexOutput {
    var out: VertexOutput;
    let model_matrix = modelBuffer[instance_id].transform;
    var worldPosition = model_matrix * vec4f(in.position, 1.0);
    if (instance_id == 0u) {
        worldPosition = model_matrix *vec4f(uUniformData.camera_world_position + in.position, 1.0);
        
    }
    out.position = uUniformData.projection_matrix * uUniformData.view_matrix * worldPosition;
    
    out.tangent = (uUniformData.model_matrix * vec4f(in.tangent, 0.0)).xyz;
	out.bitangent = (uUniformData.model_matrix * vec4f(in.bitangent, 0.0)).xyz;
    out.normal = (uUniformData.model_matrix * vec4f(in.normal, 0.0)).xyz;
    out.viewDirection = uUniformData.camera_world_position - worldPosition.xyz;
    out.color = in.color;
    out.uv = in.uv;
    out.instance_id = instance_id;
    
	return out;
}


fn calculateReflectionUVs(lightDirection: vec3f) -> vec2f {
    let L = normalize(lightDirection);
    let Pi = 3.14159265359;
    // convert to spherical coords
    let phi = asin(L.y);
    let theta = atan2(L.z, L.x);

    // map spherical coords to (0,1) uv space
    let uv = vec2f((theta + (Pi / 2.0)) / Pi, (phi / Pi));
    return uv;
}

fn perceivedLuminance(color: vec3f) -> f32 {
    return (color.r * 0.299) + (color.g * 0.587) + (color.b * 0.114);
}

@fragment
fn fs_main (in: VertexOutput) -> @location(0) vec4f {
    var normal_sample = textureSample(normal_texture, texture_sampler, in.uv).rgb;
    if (in.instance_id == 0u) {
        normal_sample = in.normal;
    }
    
    let localToWorld = mat3x3f(
        normalize(in.tangent),
        normalize(in.bitangent),
        normalize(in.normal)
    );
    
    let worldN = localToWorld * (2.0*normal_sample - 1.0);
    let N = normalize(mix(in.normal, worldN, 0.0));

    var albedo = textureSample(albedo_texture, texture_sampler, in.uv).rgb;
    albedo = mix(vec3f(1.0), albedo, 0.0);

    // var lightPositions = array(
    //     // vec3f(sin(uUniformData.time)*5.0, 2.0, cos(uUniformData.time)*5.0),
    //     // vec3f(cos(uUniformData.time)*5.0, -2.0, sin(uUniformData.time)*5.0)
    //     vec3f(5.0, 2.0, 5.0),
    //     // vec3f(5.0, -2.0, 5.0)
    // );
    let V = normalize(in.viewDirection);

    let reflectedUVs = calculateReflectionUVs(-reflect(V, N));
    let reflectedRadianceSample = textureSample(radiance_texture, texture_sampler, reflectedUVs).rgb;
    let reflectedEnvironmentSample = textureSample(environment_texture, texture_sampler, reflectedUVs).rgb;
    let normalUVs = calculateReflectionUVs(N);
    let normalEnvironmentSample = textureSample(environment_texture, texture_sampler, normalUVs).rgb;
    let normalRadianceSample = textureSample(radiance_texture, texture_sampler, normalUVs).rgb;

    let skyBoxTexture = textureSample(environment_texture, texture_sampler, in.uv).rgb;
    let skyBoxRadiance = textureSample(radiance_texture, texture_sampler, in.uv).rgb;
    if (in.instance_id == 0u) {
        // todo: make this less hacky (handle in different render pass with different shader maybe)
        return vec4f(pow(skyBoxTexture, vec3f(2.2)), 1.0);
    }

    let kh = 1.0;
    let kd = 1.0;
    let ks = 0.5;
    let ka = 0.0;

    var color = vec3f(0.0);
    for (var i: i32 = 0; i < 1; i++) { // loop for every light (we do one environment sample)
        // var L = normalize(lightPositions[i].xyz);

        // let H = normalize(L + V);

        let ambient = albedo;
        // let diffuse = perceivedLuminance(normalRadianceSample) * albedo;
        let diffuse = reflectedEnvironmentSample;
        let specular = pow(max(0.0, 1.0 - dot(V, N)), kh) * perceivedLuminance(reflectedRadianceSample) * reflectedEnvironmentSample;
        color += (kd * diffuse) + (ks * specular) + (ka * ambient);
    }

    // color correction
    let linear_color = pow(color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
}
