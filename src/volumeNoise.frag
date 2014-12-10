#version 150




//#extension GL_ARB_texture_rectangle : require

//#extension GL_NV_float_float : require

uniform sampler2DRect texBack;

uniform sampler2DRect texFront;

uniform sampler3D volumeTexture;

out vec4 finalColor;


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
// noise functions.
// Author : Ian McEwan, Ashima Arts.
// Maintainer : ijm
// Lastmod : 20110822 (ijm)
// License : Copyright (C) 2011 Ashima Arts. All rights reserved.
// Distributed under the MIT License. See LICENSE file.
// https://github.com/ashima/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2 C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i = floor(v + dot(v, C.yyy) );
  vec3 x0 = v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  // x0 = x0 - 0.0 + 0.0 * C.xxx;
  // x1 = x0 - i1 + 1.0 * C.xxx;
  // x2 = x0 - i2 + 2.0 * C.xxx;
  // x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ ); // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }

vec3 intersect(vec3 origin, vec3 rayToLight){

    float divx = 1.0 / rayToLight.x;
    float divy = 1.0 / rayToLight.y;
    float divz = 1.0 / rayToLight.z;
    float t1x, t1y, t1z;
    if (divx >=0.0)
    {
         t1x = (1.0 - origin.x) * divx;
    }
    else
    {
         t1x = ( 0.0 - origin.x) *divx;
    }

    if (divy >=0.0)
    {
         t1y = (1.0 - origin.y) * divy;
    }
    else
    {
         t1y = ( 0.0 - origin.y) *divy;
    }

    if (divz >=0.0)
    {
         t1z = (1.0 - origin.z) * divz;
    }
    else
    {
         t1z = ( 0.0 - origin.z) *divz;
    }




    return vec3(t1x, t1y, t1z);



}

bool insideBounds(vec3 position){

    if (position.x >= 0.999 || position.x <= 0.01 || position.y >= 0.999 || position.y <= 0.01 || position.z >= 0.9999 || position.z <= 0.01 )
    {

    return false;

    }

    else return true;

}

void main()

    {

    // diagonal of the cube

        float maxDist = sqrt(3.0);

        float g_lightIntensity = 2.0;

        float g_absorption = 2;
        //float g_light_absorbtion = g_absorption/1.5;
        float g_light_absorbtion = 1; //shadow strength?
        vec3 g_lightPos = vec3(-2, 3, -3);

        vec3 emissive = vec3(1.0, 0.1, 0.1);

        // assume all coordinates are in texture space

        vec4 frontPos = texture(texFront, gl_FragCoord.xy);

        vec4 backPos = texture(texBack, gl_FragCoord.xy);
        vec3 pos = (frontPos.xyz);

        vec3 eyeRay =   backPos.xyz - frontPos.xyz ;

        vec3 eyeDir = normalize( eyeRay);
        float mag = length(eyeRay);
        int numSamples = 256;
        int numLightSamples =125;

        float scale = mag/float(numSamples);
        float stepsize = 0.014;

        float densityScale = 10;
        vec3 eyeInc = eyeDir*stepsize;
       // transmittance

        float T = 1.0; //density tranmittance

        float prev_light_alpha = 0.0;

       // in-scattered radiance

        vec3 Lo = vec3(0, 0, 0);
        float La = 0.0;
        float voxColor = 1.0;

        //float CoefMult = -g_absorption*scale;



       for (int i=0; i < numSamples; i++)

            {
                    if (length(frontPos.xyz-pos.xyz) >= mag)
                    {
                        break;
                    }

                    float noisy = clamp( snoise(pos.xyz*3.2),0.0,1.0);
                    float density = noisy*densityScale;
                    //float density = snoise(pos.xyz*2)*densityScale;
                     // skip empty space
                    if (density > 0.0)
                    {
                    // attenuate ray-throughput
                        //T *= 1.0-(g_absorption*density*stepsize);

                        T *= exp(-g_absorption*density*stepsize);
                        if (T < 0.05){
                             break;
                        }


                        // point light dir in texture space
                        vec3 lightDir = normalize(g_lightPos-pos)*stepsize;

                        // sample light
                        float Tl = 1.0; // transmittance along light ray
                        vec3 lpos = pos;// + lightDir;

                        for (int s=0; s < numLightSamples; ++s)
                        {

                            float ld = clamp( snoise(lpos.xyz*3.2),0.0,1.0);
                            float densL = ld*densityScale;
                            //Tl *= 1.0-(g_light_absorbtion*stepsize*densL);
                            Tl *= exp(-g_light_absorbtion*densL*stepsize);
                            if (Tl <= 0.01)
                            break;
                            if (!insideBounds(lpos.xyz))
                            {
                            break;
                            }

                            lpos += lightDir;
                        }

                    vec3 Li = vec3(g_lightIntensity*Tl);



                    Lo += T*density*stepsize*Li;

                    }
                pos.xyz += eyeInc.xyz;
           }

       finalColor.xyz = Lo;
       finalColor.w = 1.0-T;



}
