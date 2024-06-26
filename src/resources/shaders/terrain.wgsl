struct UniformData {
    view_matrix: mat4x4f,
    projection_matrix: mat4x4f,
    camera_world_position: vec3f,
    time: f32
};

struct ModelData {
    transform: mat4x4f,
}

struct MaterialData {
    baseColor: vec3f,
    diffuseTextureIndex: u32,
    normalTextureIndex: u32,
    roughness: f32,
};

@group(0) @binding(0) var<uniform> uUniformData: UniformData;
@group(0) @binding(1) var texture_sampler: sampler;
@group(0) @binding(2) var<storage, read> modelBuffer: array<ModelData>;
@group(0) @binding(3) var<storage, read> materialBuffer: array<MaterialData>;

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

fn mapZeroToOneV(v: vec3f) -> vec3f {
    return vec3(mapZeroToOne(v.x), mapZeroToOne(v.y), mapZeroToOne(v.z));
}


fn fbmV(x: vec3f, H: f32, numOctaves: u32, initialFrequency: f32) -> vec3f {
    var G = exp2(-H);
    var f = initialFrequency;
    var a = 0.5;
    var t = vec3f(0,0,0);
    for( var i: u32 = 0 ; i < numOctaves; i++ ) {
        t += a*noise3D(x * f);
        t = clamp(t, vec3(-1.,-1.,-1.), vec3(1.,1.,1.));
        f *= 2.0;
        a *= G;
    }
    return t;
}

fn fbm(x: vec3f, H: f32, numOctaves: u32, initialFrequency: f32) -> f32 {
    var G = exp2(-H);
    var f = initialFrequency;
    var a = 0.5;
    var t = 0.;
    for( var i: u32 = 0 ; i < numOctaves; i++ ) {
        var n = a*noise3D(x * f);
        // n = n * n;
        n = clamp(n, -1., 1.);
        t += n;
        t = clamp(t, -1., 1.);
        f *= 2.0;
        a *= G;
    }
    return t;
}

@vertex
fn vs_main (in: VertexInput, @builtin(vertex_index) i: u32, @builtin(instance_index) instance_id: u32 ) -> VertexOutput {
    var out: VertexOutput;
    let modelData = modelBuffer[instance_id];
    var worldPosition = modelData.transform * vec4f(in.position, 1.0);

    let noiseInput = worldPosition.xyz;
    worldPosition.y = mapZeroToOne(fbm(noiseInput, 0.5, 8u, .01)) * 50.;
    out.position = uUniformData.projection_matrix * uUniformData.view_matrix * worldPosition;

    out.tangent = (modelData.transform * vec4f(in.tangent, 0.0)).xyz;
	out.bitangent = (modelData.transform * vec4f(in.bitangent, 0.0)).xyz;
    out.normal = (modelData.transform * vec4f(in.normal, 0.0)).xyz;
    out.viewDirection = uUniformData.camera_world_position - worldPosition.xyz;
    out.color = vec3f(worldPosition.y) / 25.;
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

    let localToWorld = mat3x3f(
        normalize(in.tangent),
        normalize(in.bitangent),
        normalize(in.normal)
    );
    
    let worldN = localToWorld * (2.0*in.normal - 1.0);
    
    // todo: need to figure out how to get the real normal
    var N = normalize(mix(worldN, in.normal, 0.));

    var lightPositions = array(
        // vec3f(sin(uUniformData.time)*5.0, 2.0, cos(uUniformData.time)*5.0),
        // vec3f(cos(uUniformData.time)*5.0, -2.0, sin(uUniformData.time)*5.0)
        vec3f(0.0, 1000.0, 0.0),
        // vec3f(5.0, -2.0, 5.0)
    );
    let V = normalize(in.viewDirection);

    let reflectedUVs = calculateReflectionUVs(-reflect(V, N));
    let normalUVs = calculateReflectionUVs(N);
    
    var albedo = in.color;

    let kh = 60.0;
    let kd = 0.8;
    let ks = 0.5;
    let ka = 0.2;

    var color = vec3f(0.0);
    for (var i: i32 = 0; i < 2; i++) { // loop for every light (we do one environment sample)
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
