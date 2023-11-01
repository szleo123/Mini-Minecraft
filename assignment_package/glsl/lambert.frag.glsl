#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.

uniform sampler2D u_Texture; // texture sampler for the shader
uniform sampler2D u_Normal; // normal map sampler for the shader

uniform int u_Time; // time

uniform vec3 u_Sun;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec4 fs_UV;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

// some global constant
vec4 WATER = vec4(0., 0., 0.75, 1.);
vec4 LAVA  = vec4(207.f, 16.f, 32.f, 255.f) / 255.f;
#define M_PI 3.1415926535897932384626433832795

void coordinateSystem(in vec3 v1, out vec3 v2, out vec3 v3) {
    if (abs(v1.x) > abs(v1.y))
            v2 = vec3(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
        else
            v2 = vec3(0, v1.z, -v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
        v3 = cross(v1, v2);
}

mat3 LocalToWorld(vec3 nor) {
    vec3 tan, bit;
    coordinateSystem(nor, tan, bit);
    return mat3(tan, bit, nor);
}

void main()
{
    // Material base color (before shading)
    vec2 uv = fs_UV.xy;
    // animation of water
    if (length(fs_Col) == length(WATER)) {
        uv.x += (cos(2 * M_PI * (u_Time / 10000.f)) + 1.f) / 16.f;
    }
    if (length(fs_Col) == length(LAVA)) {
        uv.x += (cos(2 * M_PI * (u_Time / 10000.f)) + 1.f) / 16.f;
    }
    ///////////////////////
    vec4 diffuseColor = texture2D(u_Texture, uv);
    float alpha = diffuseColor.a;
    vec4 normal = fs_Nor;
//    vec4 normal;
//    if (length(fs_Col) == length(WATER)) {
//        normal = fs_Nor;
//    } else {
//        normal = texture2D(u_Normal, fs_UV.xy);
//        normal = vec4(LocalToWorld(fs_Nor.xyz) * normal.xyz, normal.a);
//    }
    diffuseColor = diffuseColor * (0.5 * fbm(fs_Pos.xyz) + 0.5);

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(normal), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    if (u_Sun.y < 0 && length(fs_Col) != length(LAVA)) {
        diffuseTerm = 0.;
    }

    float ambientTerm = 0.2;

    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.

    // Compute final shaded color
    out_Col = vec4(diffuseColor.rgb * lightIntensity, alpha);

}
