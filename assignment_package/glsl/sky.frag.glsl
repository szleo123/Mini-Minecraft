#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform float u_Time;

uniform vec3 u_Sun;

uniform vec3 u_Player;

out vec4 out_Col;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                               vec3(254, 192, 81) / 255.0,
                               vec3(255, 137, 103) / 255.0,
                               vec3(253, 96, 81) / 255.0,
                               vec3(57, 32, 51) / 255.0);
// Dusk palette
const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
                            vec3(96, 72, 120) / 255.0,
                            vec3(72, 48, 120) / 255.0,
                            vec3(48, 24, 96) / 255.0,
                            vec3(0, 24, 72) / 255.0);

const vec3 daylight[5] = vec3[](vec3(254, 216, 0) / 255.0,
                                vec3(255, 247, 0) / 255.0,
                                vec3(255, 252, 193) / 255.0,
                                vec3(119, 220, 239) / 255.0,
                                vec3(1, 203, 254) / 255.0);

const vec3 nightlight[5] = vec3[](vec3(190,169,222) / 255.0,
                                vec3(135,136,156) / 255.0,
                                vec3(84,107,171) / 255.0,
                                vec3(46,68,130) / 255.0,
                                vec3(19,24,98) / 255.0);

const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 cloudColor = sunset[3];

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.5) {
        return sunset[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    return sunset[4];
}

vec3 uvToDusk(vec2 uv) {
    if(uv.y < 0.5) {
        return dusk[0];
    }
    else if(uv.y < 0.55) {
        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
    }
    return dusk[4];
}

vec3 uvToDaylight(vec2 uv) {
    if(uv.y < 0.35) {
        return daylight[0];
    }
    else if(uv.y < 0.4) {
        return mix(daylight[0], daylight[1], (uv.y - 0.35) / 0.05);
    }
    else if(uv.y < 0.50) {
        return mix(daylight[1], daylight[2], (uv.y - 0.4) / 0.1);
    }
    else if(uv.y < 0.60) {
        return mix(daylight[2], daylight[3], (uv.y - 0.5) / 0.1);
    }
    else if(uv.y < 0.70) {
        return mix(daylight[3], daylight[4], (uv.y - 0.6) / 0.1);
    }
    return daylight[4];
}

vec3 uvToNightlight(vec2 uv) {
    if(uv.y < 0.35) {
        return nightlight[0];
    }
    else if(uv.y < 0.4) {
        return mix(nightlight[0], nightlight[1], (uv.y - 0.35) / 0.05);
    }
    else if(uv.y < 0.50) {
        return mix(nightlight[1], nightlight[2], (uv.y - 0.4) / 0.1);
    }
    else if(uv.y < 0.60) {
        return mix(nightlight[2], nightlight[3], (uv.y - 0.5) / 0.1);
    }
    else if(uv.y < 0.70) {
        return mix(nightlight[3], nightlight[4], (uv.y - 0.6) / 0.1);
    }
    return nightlight[4];
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.000001 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}

//#define RAY_AS_COLOR
//#define SPHERE_UV_AS_COLOR
#define WORLEY_OFFSET

void main()
{
    vec3 outColor = vec3(0.);
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC
//    out_Col = vec4(ndc * 0.5 + 0.5, 1, 1);

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);

#ifdef RAY_AS_COLOR
    out_Col = vec4(0.5 * (rayDir + vec3(1,1,1)), 1);
    return;
#endif

    vec2 uv = sphereToUV(rayDir);
#ifdef SPHERE_UV_AS_COLOR
    out_Col = vec4(uv, 0, 1);
    return;
#endif


    vec2 offset = vec2(0.0);
#ifdef WORLEY_OFFSET
    // Get a noise value in the range [-1, 1]
    // by using Worley noise as the noise basis of FBM
    offset = vec2(worleyFBM(rayDir));
    offset *= 2.0;
    offset -= vec2(1.0);
#endif

    // Compute a gradient from the bottom of the sky-sphere to the top
    vec3 sunsetColor = uvToSunset(uv + offset * 0.1);
    vec3 duskColor = uvToDusk(uv + offset * 0.1);
    vec3 daylightColor = uvToDaylight(uv + offset * 0.1);
    vec3 nightlightColor = uvToNightlight(uv + offset * 0.1);

    outColor = sunsetColor;

    // Add a glowing sun in the sky
//    vec3 sunDir = normalize(vec3(0, 0.1, 1.0));
//    vec3 sunDir = normalize(vec3(-1, 0, .0));
    vec3 sunDir = vec3(normalize(u_Sun.xy), 0.);
    float sunrise_t = 0.25;
    float sunrise2_t = 0.35;
    float noon_t = 2.5;
    float noon2_t = 2.8;
    float night_t = -0.5;
    float night2_t = -2.6;
    float checker = atan(sunDir.y, sunDir.x);
#define SUNRISE_THRESHOLD 0.85
#define NIGHT_THRESHOLD -0.1
#define SUNSET_THRESHOLD 0.75
#define DUSK_THRESHOLD -0.1
    if (checker >= 0 && checker < sunrise_t) {
        float raySunDot = dot(rayDir, sunDir);
        if (raySunDot > SUNRISE_THRESHOLD) {
            outColor = daylightColor;
        } else if (raySunDot > NIGHT_THRESHOLD) {
            float t = (raySunDot - SUNRISE_THRESHOLD) / (NIGHT_THRESHOLD - SUNRISE_THRESHOLD);
            outColor = mix(daylightColor, nightlightColor, t);
        } else {
            outColor = nightlightColor;
        }
    } else if (checker >= sunrise_t && checker < sunrise2_t) {
        float raySunDot = dot(rayDir, sunDir);
        if (raySunDot > SUNRISE_THRESHOLD) {
            outColor = daylightColor;
        } else if (raySunDot > NIGHT_THRESHOLD) {
            float t = (raySunDot - SUNRISE_THRESHOLD) / (NIGHT_THRESHOLD - SUNRISE_THRESHOLD);
            outColor = mix(daylightColor, nightlightColor, t);
        } else {
            outColor = nightlightColor;
        }
        float t2 = (checker - sunrise_t) / (sunrise2_t - sunrise_t);
        outColor = mix(outColor, daylightColor, t2);
    } else if (checker >= sunrise2_t && checker < noon_t) {
        outColor = daylightColor;
    } else if (checker >= noon_t && checker < noon2_t) {
        float raySunDot = dot(rayDir, sunDir);

        if(raySunDot > SUNSET_THRESHOLD) {
            // Do nothing, sky is already correct color
            outColor = sunsetColor;
        }
        // Any dot product between 0.75 and -0.1 is a LERP b/t sunset and dusk color
        else if(raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            outColor = mix(outColor, duskColor, t);
        }
        // Any dot product <= -0.1 are pure dusk color
        else {
            outColor = duskColor;
        }
        float t2 = (checker - noon_t) / (noon2_t - noon_t);
        outColor = mix(daylightColor, outColor, t2);
    } else if (checker >= noon2_t && checker < PI) {
        float raySunDot = dot(rayDir, sunDir);
        if(raySunDot > SUNSET_THRESHOLD) {
            // Do nothing, sky is already correct color
            outColor = sunsetColor;
        }
        // Any dot product between 0.75 and -0.1 is a LERP b/t sunset and dusk color
        else if(raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            outColor = mix(outColor, duskColor, t);
        }
        // Any dot product <= -0.1 are pure dusk color
        else {
            outColor = duskColor;
        }
    } else if (checker < 0 && checker > night_t) {
        float raySunDot = dot(rayDir, sunDir);
        if (raySunDot > SUNRISE_THRESHOLD) {
            outColor = daylightColor;
        } else if (raySunDot > NIGHT_THRESHOLD) {
            float t = (raySunDot - SUNRISE_THRESHOLD) / (NIGHT_THRESHOLD - SUNRISE_THRESHOLD);
            outColor = mix(daylightColor, nightlightColor, t);
        } else {
            outColor = nightlightColor;
        }
        float t2 = (checker) / (night_t);
        outColor = mix(outColor, nightlightColor, t2);
    } else if (checker <= night_t && checker > night2_t) {
        outColor = nightlightColor;
    } else {
        float raySunDot = dot(rayDir, sunDir);
        if(raySunDot > SUNSET_THRESHOLD) {
            // Do nothing, sky is already correct color
            outColor = sunsetColor;
        }
        // Any dot product between 0.75 and -0.1 is a LERP b/t sunset and dusk color
        else if(raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            outColor = mix(outColor, duskColor, t);
        }
        // Any dot product <= -0.1 are pure dusk color
        else {
            outColor = duskColor;
        }
        float t2 = (checker + PI) / (PI + night2_t);
        outColor = mix(outColor, nightlightColor, t2);
    }
    out_Col = vec4(outColor, 1);
}
