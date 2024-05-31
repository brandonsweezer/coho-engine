struct UniformData {
    model_matrix: mat4x4f,
    view_matrix: mat4x4f,
    projection_matrix: mat4x4f,
    camera_world_position: vec3f,
    time: f32
};

struct ModelData {
    transform: mat4x4f,
    materialIndex: u32,
    isSkybox: u32,
}

struct MaterialData {
    baseColor: vec3f,
    diffuseTextureIndex: u32,
    normalTextureIndex: u32,
    roughness: f32,
};

@group(0) @binding(0) var<uniform> uUniformData: UniformData;
@group(0) @binding(1) var textureArray: binding_array<texture_2d<f32>>;
@group(0) @binding(2) var texture_sampler: sampler;
@group(0) @binding(3) var<storage, read> modelBuffer: array<ModelData>;
@group(0) @binding(4) var<storage, read> materialBuffer: array<MaterialData>;

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
    let modelData = modelBuffer[instance_id];
    var worldPosition = modelData.transform * vec4f(in.position, 1.0);
    if (modelData.isSkybox == 1u) {
        worldPosition = modelData.transform * vec4f(uUniformData.camera_world_position + in.position, 1.0);
    }
    out.position = uUniformData.projection_matrix * uUniformData.view_matrix * worldPosition;
    
    out.tangent = (modelData.transform * vec4f(in.tangent, 0.0)).xyz;
	out.bitangent = (modelData.transform * vec4f(in.bitangent, 0.0)).xyz;
    out.normal = (modelData.transform * vec4f(in.normal, 0.0)).xyz;
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
    let modelData = modelBuffer[in.instance_id];
    let materialData = materialBuffer[modelData.materialIndex];
    var normal_sample = textureSample(textureArray[materialData.normalTextureIndex], texture_sampler, in.uv).rgb;
    if (modelData.isSkybox == 1u) {
        normal_sample = in.normal;
    }

    let localToWorld = mat3x3f(
        normalize(in.tangent),
        normalize(in.bitangent),
        normalize(in.normal)
    );
    
    let worldN = localToWorld * (2.0*normal_sample - 1.0);
    let normalMixBool = i32(materialData.normalTextureIndex) == -1;
    let normalMix = f32(normalMixBool);
    let N = normalize(mix(worldN, in.normal, normalMix));

    var lightPositions = array(
        vec3f(sin(uUniformData.time)*5.0, 2.0, cos(uUniformData.time)*5.0),
    //     // vec3f(cos(uUniformData.time)*5.0, -2.0, sin(uUniformData.time)*5.0)
        // vec3f(5.0, 2.0, 5.0),
    //     // vec3f(5.0, -2.0, 5.0)
    );
    let V = normalize(in.viewDirection);

    let reflectedUVs = calculateReflectionUVs(-reflect(V, N));
    let normalUVs = calculateReflectionUVs(N);
    
    var albedo = textureSample(textureArray[materialData.diffuseTextureIndex], texture_sampler, in.uv).rgb;
    let albedoMixBool = i32(materialData.diffuseTextureIndex) == -1;
    let albedoMix = f32(albedoMixBool);
    albedo = mix(albedo, materialData.baseColor, albedoMix);

    if (modelData.isSkybox == 1u) {
        return vec4f(pow(albedo, vec3f(2.2)), 1.0);
    }

    let kh = 1.0;
    let kd = 1.0;
    let ks = 0.0;
    let ka = 0.0;

    var color = vec3f(0.0);
    for (var i: i32 = 0; i < 1; i++) { // loop for every light (we do one environment sample)
        var L = normalize(lightPositions[i].xyz);

        // let H = normalize(L + V);

        let ambient = albedo;
        let diffuse = max(0.0, dot(N, L)) * albedo;
        // let diffuse = reflectedEnvironmentSample;
        let specular = vec3f(0.0,0.0,0.0);
        // let specular = pow(max(0.0, 1.0 - dot(V, N)), kh) * perceivedLuminance(reflectedRadianceSample) * reflectedEnvironmentSample;
        color += (kd * diffuse) + (ks * specular) + (ka * ambient);
    }

    // color correction
    let linear_color = pow(color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
}
