#version 150


out vec4 FragColor;


uniform sampler2DRect texBack;

uniform sampler2DRect texFront;

uniform sampler3D volumeTexture;

vec3 LightPosition = vec3(-1.0, 1.0, 3.0);
vec3 LightIntensity = vec3(3.0);
float Absorption = 0.5;


const float maxDist = sqrt(2.0);
const int numSamples = 256;
const float stepSize = maxDist/float(numSamples);
const int numLightSamples = 64;
const float lscale = maxDist / float(numLightSamples);
const float densityFactor = 1;

float GetDensity(vec3 pos)
{
    return texture(volumeTexture, pos).x * densityFactor;
}

struct Ray {
    vec3 Origin;
    vec3 Dir;
};

struct AABB {
    vec3 Min;
    vec3 Max;
};

bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1)
{
    vec3 invR = 1.0 / r.Dir;
    vec3 tbot = invR * (aabb.Min-r.Origin);
    vec3 ttop = invR * (aabb.Max-r.Origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

void main()
{
    vec3 rayDirection;

    vec4 frontPos = texture(texFront, gl_FragCoord.xy);

    vec4 backPos = texture(texBack, gl_FragCoord.xy);

    rayDirection =   backPos.xyz - frontPos.xyz ;

    float travel = distance(backPos, frontPos);
   // float stepSize = travel/numSamples;

    vec3 pos = frontPos.xyz;
    vec3 step = normalize(rayDirection) * stepSize;
    float T = 1.0;
    vec3 Lo = vec3(0.0);


    for (int i=0; i < numSamples && travel > 0.0; ++i, pos += step, travel -= stepSize) {

        float density = GetDensity(pos);
        if (density <= 0.0)
            continue;

        T *= 1.0-density*stepSize*Absorption;

        if (T <= 0.01)
            break;

        vec3 lightDir = normalize(LightPosition-pos)*lscale;
        float Tl = 1.0;
        vec3 lpos = pos + lightDir;

        for (int s=0; s < numLightSamples; ++s) {
            float ld = texture(volumeTexture, lpos).x;
            Tl *= 1.0-Absorption*stepSize*ld;
            if (Tl <= 0.01)
            lpos += lightDir;
        }

        vec3 Li = LightIntensity*Tl;
        //Lo += Li*T*density*stepSize;
        Lo += Li*T*density*stepSize;
    }

    FragColor.rgb = Lo;
    FragColor.a = 1-(T);
}
