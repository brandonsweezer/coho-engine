struct UniformData {
    frequency: f32,
    scale: f32,
    seed: u32,
    octaves: u32
}

@group(0) @binding(0) var<storage, read> input_buffer: array<f32, 3145728>;
@group(0) @binding(1) var<storage, read_write> output_buffer: array<f32, 3145728>;
@group(0) @binding(2) var<uniform> uniformData: UniformData;

fn noise3D(v: vec3<f32>) -> f32 {
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

fn fbm(x: vec3f, H: f32, numOctaves: u32, initialFrequency: f32) -> f32 {
    var G = exp2(-H);
    var f = initialFrequency;
    var a = 1.0;
    var t = 0.0;
    for( var i: u32 = 0 ; i < numOctaves; i++ ) {
        t += a*noise3D(x * f);
        t = clamp(t, -1., 1.);
        f *= 2.0;
        a *= G;
    }
    return t;
}

fn mapZeroToOne(f: f32) -> f32 {
    return (f + 1.0) / 2.0;
}

@compute @workgroup_size(32, 32, 1)
fn main(@builtin(local_invocation_id) id: vec3<u32>, @builtin(workgroup_id) wid: vec3<u32>, @builtin(num_workgroups) nwg: vec3<u32>) {
    let workGroupSize = vec3<u32>(32u, 32u, 1u);
    let globalPos = wid.xyz * workGroupSize + id.xyz;
    let globalIndex = globalPos.y * workGroupSize.x * nwg.x + globalPos.x;
    var globalPosF = vec3<f32>(
        f32(globalPos.x),
        f32(globalPos.y),
        f32(uniformData.seed),
    );

    var value = fbm(globalPosF, 0.5, 10u, 0.01);

    output_buffer[globalIndex] = mapZeroToOne(value);
}