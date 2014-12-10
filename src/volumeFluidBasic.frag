#version 150




//#extension GL_ARB_texture_rectangle : require

//#extension GL_NV_float_float : require

uniform sampler2DRect texBack;

uniform sampler2DRect texFront;

uniform sampler3D volumeTexture;

out vec4 finalColor;




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


        float g_absorption = 0.5;
        vec3 g_lightPos = vec3(2, 3, 3);



        vec4 frontPos = texture(texFront, gl_FragCoord.xy);

        vec4 backPos = texture(texBack, gl_FragCoord.xy);
        vec3 eyeRay =   backPos.xyz - frontPos.xyz ;
        vec3 eyeDir = normalize( eyeRay);

        vec3 pos = (frontPos.xyz)+eyeDir*0.05;


        float mag = length(eyeRay);
        int numSamples = 125;
        int numLightSamples =125;

        float scale = mag/float(numSamples-1);
        float stepsize = 0.014;

        float densityScale = 1;
        vec3 eyeInc = eyeDir*stepsize;
       // transmittance

        float T = 1.0; //density tranmittance


        vec3 Lo = vec3(0, 0, 0);



       for (int i=0; i < numSamples-1; i++)

            {
                    float density =  texture(volumeTexture,pos).r * densityScale;

                    //float density = snoise(pos.xyz*2)*densityScale;
                     // skip empty space
                    if (density > 0.0)
                    {
                    // attenuate ray-throughput
                        //T *= 1.0-(g_absorption*density*stepsize);

                        T *= exp(-g_absorption*density*scale);
                        if (T < 0.1){
                             break;
                        }



                    Lo += T*density*scale;

                    }

                pos.xyz += eyeDir*scale;
           }

        finalColor.xyz = Lo;
        //finalColor.xyz = eyeRay+vec3(1);
       //finalColor.xyz = texture(volumeTexture,vec3(0.001)).xyz;
       //finalColor.xyz = texture(texFront, gl_FragCoord.xy).xyz;
       finalColor.w = 1.0-T;
       //finalColor.w = 1.0;




}
