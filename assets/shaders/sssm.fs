#version 410 core

out float FragColor;
in vec2 TexCoord;

struct Light {
    vec3 Position;
    vec3 Color;
    float far_plane;
    float constant;
    float Linear;
    float Quadratic;
};

uniform sampler2D gPosition; // FragPos
uniform samplerCube depthMap;
uniform vec3 viewPos;
uniform mat4 viewInv;
uniform float depthBias;
uniform float diskFactor;
uniform int LightNum;
uniform Light lights[1];

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float ShadowCalculation_Soft(vec3 fragPos, Light light)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.Position;
    // Use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // It is currently in linear range between [0,1]. Let's re-transform it back to original depth value
    // closestDepth *= light.far_plane;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / light.far_plane)) / diskFactor;
    float closestDepth;
    for(int i = 0; i < samples; ++i)
    {
        closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= light.far_plane;   // Undo mapping [0;1]
        if(currentDepth - depthBias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}

float ShadowCalculation_Hard(vec3 fragPos, Light light)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.Position;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= light.far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;        

    return shadow;
}

void main()
{		
     // retrieve data from gbuffer
    vec3 FragPos = (viewInv * vec4(texture(gPosition, TexCoord).rgb, 1.0)).xyz;
    float shadow = ShadowCalculation_Hard(FragPos, lights[0]);
    float brightness = 1 - shadow;
    FragColor =  brightness / (1 + brightness);
}