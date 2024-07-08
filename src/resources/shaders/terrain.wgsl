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
    @location(0) worldPosition: vec4f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
    @location(3) viewDirection: vec3f,
    @location(4) tangent: vec3f,
    @location(5) bitangent: vec3f,
    @location(6) uv: vec2f,
    @location(7) instance_id: u32
}

fn voronoise(v: vec2<f32>) -> vec2f {
    return vec2(sin(v.x) * cos(v.y), sin(v.y) * cos(v.x));
}

fn voronoi(p: vec2<f32>) -> f32 {
    let i = floor(p);
    let f = fract(p);

    var minDist = 2.;
    var closestPoint = vec2(0.);

    var noiseInput = vec3(0., 0., 0.);
    for (var x: f32 = -1.; x <= 1.; x = x + 1.) {
        for (var y: f32 = -1.; y <= 1.; y = y + 1.) {
            let neighbor = vec2(x, y);

            noiseInput = vec3<f32>((neighbor + i).x, (neighbor + i).y, 0.);
            let point = vec2(noise3D(noiseInput), noise3D(noiseInput * 1000));
            let diff = neighbor + vec2(point) - f;
            let dist = length(diff);
            if (dist <= minDist) {
                minDist = dist;
                closestPoint = point;
            }
        }
    }
    return 1. - minDist;
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

fn getHeight(noiseInput: vec3<f32>) -> f32 {
    return mapZeroToOne(fbm(noiseInput, 0.9, 8u, .001)) * 1000.0 - 500.0;
}

@vertex
fn vs_main (in: VertexInput, @builtin(vertex_index) i: u32, @builtin(instance_index) instance_id: u32 ) -> VertexOutput {
    var out: VertexOutput;
    let modelData = modelBuffer[instance_id];
    var worldPosition = modelData.transform * vec4f(in.position, 1.0);

    let noiseInput = worldPosition.xyz;
    worldPosition.y = mapZeroToOne(fbm(noiseInput, 0.9, 8u, .001)) * 1000.0 - 500.0;
    out.position = uUniformData.projection_matrix * uUniformData.view_matrix * worldPosition;

    out.worldPosition = worldPosition;
    out.viewDirection = uUniformData.camera_world_position - worldPosition.xyz;
    
    out.color = mapToGradient((worldPosition.y + 500) / 1000.0);
    out.uv = in.uv;
    out.instance_id = instance_id;
    
	return out;
}

fn mapToGradient(value: f32) -> vec3f {
    let color1 = vec3f(0.475, .630, 0.0504);
    let color2 = vec3f(1.0, 1.0, 1.0);
    let topThreshold = 0.1;
    let bottomThreshold = 0.65;
    
    // Determine where the value falls in the range
    if (value <= topThreshold) {
        return color1;
    } else if (value >= bottomThreshold) {
        return color2;
    } else {
        // Interpolate between color1 and color2 for values in between
        let t = (value - topThreshold) / (bottomThreshold - topThreshold);
        return mix(color1, color2, t);
    }
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
    // let noiseInput = in.worldPosition.xyz;

    // calculate derivative
    let delta = .01;
    let noiseInput = in.worldPosition.xyz + vec3<f32>(in.uv.x, 0, in.uv.y);
    // Sample the heights at surrounding positions
    let heightL = getHeight(noiseInput + vec3<f32>(-delta, 0.0, 0.0));
    let heightR = getHeight(noiseInput + vec3<f32>( delta, 0.0, 0.0));
    let heightD = getHeight(noiseInput + vec3<f32>(0.0, 0.0, -delta));
    let heightU = getHeight(noiseInput + vec3<f32>(0.0, 0.0,  delta));
    let posL = in.worldPosition.xyz + vec3<f32>(-delta, heightL + in.worldPosition.y, 0.0) + vec3<f32>(in.uv.x, 0, in.uv.y);
    let posR = in.worldPosition.xyz + vec3<f32>( delta, heightR + in.worldPosition.y, 0.0) + vec3<f32>(in.uv.x, 0, in.uv.y);
    let posD = in.worldPosition.xyz + vec3<f32>(0.0, heightD + in.worldPosition.y, -delta) + vec3<f32>(in.uv.x, 0, in.uv.y);
    let posU = in.worldPosition.xyz + vec3<f32>(0.0, heightU + in.worldPosition.y,  delta) + vec3<f32>(in.uv.x, 0, in.uv.y);

    // out.normal = normalize(vec3<f32>(heightL - heightR, 2 * delta, heightD - heightU));
    let tangent = normalize(posR - posL);
    let bitangent = normalize(posU - posD);
    var normal = normalize(vec3<f32>(heightL - heightR, 2 * delta, heightD - heightU));

    let bump = vec3f(0.,
        voronoi(in.uv * 10),
        0.
    );
    
    let localToWorld = mat3x3f(
        tangent,
        bitangent,
        normal
    );

    normal = localToWorld * vec3f(0., 1., 0.);

    let lightDir = normalize(vec3(1.,1.,1.));
    let diffuse1 = max(dot(normal, lightDir), 0.);
    let diffuse2 = max(dot(normal, vec3(-0.2,1.,-0.4)), 0.);
    let sunColor = vec3f(1., .98, .95);
    let diffuseColor = diffuse1 * in.color * sunColor;
    var fogfactor = clamp(distance(uUniformData.camera_world_position, in.worldPosition.xyz) / 5000., 0., 1.);
    fogfactor = 0.;
    var color = mix(diffuseColor, vec3f(.6, .6, 0.65), fogfactor);
    // color = normal;
    // let color = in.normal * 0.5 + vec3(0.5);
    let linear_color = pow(color, vec3f(2.2));
    return vec4f(linear_color, 1.0);
}
