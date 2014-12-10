#version 150
//#extension GL_ARB_texture_rectangle : require

uniform sampler2DRect texBack;
uniform sampler2DRect texFront;

in vec3 outColour;

out vec4 finalColor;

void main()
    {
    // diagonal of the cube
        float maxDist = sqrt(3.0);
        float g_lightIntensity = 0.5;
        float g_absorption = 0.5;
        vec3 g_lightPos = vec3(3.0, 0.0, 3.0);
        int numSamples = 128;
        float scale = maxDist/float(numSamples);
        int numLightSamples = 32;
        float lscale = maxDist / float(numLightSamples);                                         
        // assume all coordinates are in texture space                                                 
        vec4 frontPos = texture(texFront, gl_FragCoord.xy);
        vec4 backPos = texture(texBack, gl_FragCoord.xy);
        vec3 eyeRay = backPos.xyz - frontPos.xyz;
        vec3 pos = frontPos.xyz;
        vec3 eyeDir = normalize(pos.xyz-eyeRay.xyz)*scale;
        
        // transmittance                                                                               
        float T = 1.0;                                                                                 
        // in-scattered radiance                                                                       
        vec3 Lo = vec3(0.0);
        
        for (int i=0; i < numSamples; ++i)                                                             
            {                                                                                          
                // sample density                                                                      
                float density = 0.5;
                // skip empty space                                                                    
                if (density > 0.0)                                                                     
                    {                                                                                  
                        // attenuate ray-throughput                                                    
                        T *= 1.0-density*scale*g_absorption;                                           
                        if (T <= 0.01)                                                                 
                            {                                                                          
                                break;                                                                 
                            }                                                                          
                        // point light dir in texture space                                            
                        vec3 lightDir = normalize(g_lightPos.xyz-pos.xyz)*lscale;
                        // sample light                                                                
                        float Tl = 1.0; // transmittance along light ray                               
                        vec3 lpos = pos.xyz + lightDir.xyz;
                        for (int s=0; s < numLightSamples; ++s)                                        
                            {                                                                          
                                float ld = 0.5; ;                            
                                Tl *= 1.0-g_absorption*lscale*ld;                                      
                                if (Tl <= 0.01)                                                        
                                    {                                                                  
                                        break;                                                         
                                    }                                                                  
                                lpos += lightDir;                                                      
                            }                                                                          
                        vec3 Li = vec3(g_lightIntensity*Tl);
                        Lo += Li+T*density*scale;
                    }                                                                                          
                pos.xyz += eyeDir.xyz;
            }                                                                                          
        finalColor.xyz = Lo;
        //finalColor.xyz = vec3(0.5);

        finalColor.w = 1.0-T;
}


