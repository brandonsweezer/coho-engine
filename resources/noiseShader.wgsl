struct UniformData {
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

@group(0) @binding(0) var<uniform> uUniformData: UniformData;
@group(0) @binding(1) var textureArray: binding_array<texture_2d<f32>>;
@group(0) @binding(2) var texture_sampler: sampler;
@group(0) @binding(3) var<storage, read> modelBuffer: array<ModelData>;
@group(0) @binding(4) var<storage, read> materialBuffer: array<MaterialData>;

fn noise3D(v: vec3<f32>) -> f32 {
    // let v = pos * 0.0001;
    let F3 = 1.0 / 3.0;
    let G3 = 1.0 / 6.0;

    // Skew the input space to determine which simplex cell we're in
    let s = (v.x + v.y + v.z) * F3;
    let i = i32(floor(v.x + s));
    let j = i32(floor(v.y + s));
    let k = i32(floor(v.z + s));

    // Unskew the cell origin back to (x,y,z) space
    let t = f32(i + j + k) * G3;
    let X0 = vec3<f32>(f32(i), f32(j), f32(k)) - vec3<f32>(t, t, t);
    let x0 = v - X0;

    // Determine which simplex we are in
    var i1 = vec3<i32>(0, 0, 0);
    var j1 = vec3<i32>(0, 0, 0);
    var k1 = vec3<i32>(0, 0, 0);
    
    if (x0.x >= x0.y) {
        if (x0.y >= x0.z) {
            i1 = vec3<i32>(1, 0, 0);
            j1 = vec3<i32>(1, 1, 0);
            k1 = vec3<i32>(1, 1, 1);
        } else if (x0.x >= x0.z) {
            i1 = vec3<i32>(1, 0, 0);
            j1 = vec3<i32>(1, 0, 1);
            k1 = vec3<i32>(1, 1, 1);
        } else {
            i1 = vec3<i32>(0, 0, 1);
            j1 = vec3<i32>(1, 0, 1);
            k1 = vec3<i32>(1, 1, 1);
        }
    } else {
        if (x0.y < x0.z) {
            i1 = vec3<i32>(0, 0, 1);
            j1 = vec3<i32>(0, 1, 1);
            k1 = vec3<i32>(1, 1, 1);
        } else if (x0.x < x0.z) {
            i1 = vec3<i32>(0, 1, 0);
            j1 = vec3<i32>(0, 1, 1);
            k1 = vec3<i32>(1, 1, 1);
        } else {
            i1 = vec3<i32>(0, 1, 0);
            j1 = vec3<i32>(1, 1, 0);
            k1 = vec3<i32>(1, 1, 1);
        }
    }

    // Offsets for the middle and last corners
    let x1 = x0 - vec3<f32>(i1) + vec3<f32>(G3, G3, G3);
    let x2 = x0 - vec3<f32>(j1) + vec3<f32>(2.0 * G3, 2.0 * G3, 2.0 * G3);
    let x3 = x0 - vec3<f32>(1.0, 1.0, 1.0) + vec3<f32>(3.0 * G3, 3.0 * G3, 3.0 * G3);

    // Calculate hashed gradient indices of the four simplex corners
    let g0 = hash3D(i, j, k) % 16;
    let g1 = hash3D(i + i1.x, j + i1.y, k + i1.z) % 16;
    let g2 = hash3D(i + j1.x, j + j1.y, k + j1.z) % 16;
    let g3 = hash3D(i + 1, j + 1, k + 1) % 16;

    // Calculate the contribution from the four corners
    var n0: f32 = 0.0;
    var n1: f32 = 0.0;
    var n2: f32 = 0.0;
    var n3: f32 = 0.0;

    let t0 = 0.6 - dot(x0, x0);
    if (t0 >= 0.0) {
        let t0_4 = t0 * t0 * t0 * t0;
        n0 = t0_4 * dot(grad3(g0), x0);
    }

    let t1 = 0.6 - dot(x1, x1);
    if (t1 >= 0.0) {
        let t1_4 = t1 * t1 * t1 * t1;
        n1 = t1_4 * dot(grad3(g1), x1);
    }

    let t2 = 0.6 - dot(x2, x2);
    if (t2 >= 0.0) {
        let t2_4 = t2 * t2 * t2 * t2;
        n2 = t2_4 * dot(grad3(g2), x2);
    }

    let t3 = 0.6 - dot(x3, x3);
    if (t3 >= 0.0) {
        let t3_4 = t3 * t3 * t3 * t3;
        n3 = t3_4 * dot(grad3(g3), x3);
    }

    // Add contributions from each corner to get the final noise value
    return 16 * (n0 + n1 + n2 + n3);
}

fn hash3D(x: i32, y: i32, z: i32) -> i32 {
    var h = x * 374761393 + y * 668265263 + z * 1274126177; // Arbitrary constants
    h = (h ^ (h >> 13)) * 1274126177;
    return h ^ (h >> 16);
}

fn grad3(hash: i32) -> vec3<f32> {
    let h = hash & 15;
    var grad = vec3<f32>(1.0, 1.0, 1.0);
    if (h & 8) != 0 { grad.x = -1.0; }
    if (h & 4) != 0 { grad.y = -1.0; }
    if (h & 2) != 0 { grad.z = -1.0; }
    return grad;
}

fn mapZeroToOne(f: f32) -> f32 {
    return (f + 1.0) / 2.0;
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

fn fbm(x: vec3f, H: f32, numOctaves: u32) -> vec3f {
    var G = exp2(-H);
    var f = 0.001;
    var a = 1.0;
    var t = vec3f(0,0,0);
    for( var i: u32 = 0 ; i < numOctaves; i++ ) {
        t += a*noise3D(x * f);
        f *= 2.0;
        a *= G;
    }
    return t;
}

@fragment
fn fs_main (in: VertexOutput) -> @location(0) vec4f {
    var globalPosF = in.position.xyz;

    var color = fbm(globalPosF, 0.5, 10u);
    color = vec3(mapZeroToOne(color.x), mapZeroToOne(color.y), mapZeroToOne(color.z));

    // color correction
    let linear_color = pow(color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
}