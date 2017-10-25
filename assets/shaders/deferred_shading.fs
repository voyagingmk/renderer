#version 410 core

out vec4 FragColor;

in vec2 TexCoord;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
}; 

uniform sampler2D gPosition; // FragPos
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBR;
uniform sampler2D ssao;
uniform samplerCube depthMap;

struct Light {
    vec3 Position;
    vec3 Color;
    float far_plane;
    float Linear;
    float Quadratic;
};
uniform int LightNum;
const int MAX_LIGHTS = 32;
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 viewInv;


const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);


float ShadowCalculation(vec3 fragPos, Light light)
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
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / light.far_plane)) / 25.0;
    float closestDepth;
    for(int i = 0; i < samples; ++i)
    {
        closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= light.far_plane;   // Undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}


void main()
{		
     // retrieve data from gbuffer
    vec3 FragPos = (viewInv * vec4(texture(gPosition, TexCoord).rgb, 1.0)).xyz;
    vec3 Normal = (viewInv * vec4(normalize(texture(gNormal, TexCoord).rgb), 0.0)).xyz;
    float AmbientOcclusion = texture(ssao, TexCoord).r;
    Material material;
    material.albedo = texture(gAlbedo, TexCoord).rgb;
    vec3 pbrData = texture(gPBR, TexCoord).rgb;
    material.metallic = pbrData.r;
    material.roughness = pbrData.g;
    material.ao = pbrData.b;

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material.albedo, material.metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < LightNum; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].Position - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
        // attenuation = 1.0 / (distance * distance);
        attenuation = 1.0;
        vec3 radiance = lights[i].Color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, material.roughness);   
        float G   = GeometrySmith(N, V, L, material.roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 brdf = nominator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - material.metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * material.albedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    float shadow = ShadowCalculation(FragPos, lights[0]);

    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * material.albedo * material.ao;
    
    vec3 color = ambient + Lo * (1.0 - shadow);

    color *= AmbientOcclusion;

    // HDR tonemapping
    // color = color / (color + vec3(1.0));
    // gamma correct
    // 0.5 gamma-corrected, 就是线性空间的值
    // 0.5**(1/2.2) = 0.729（变亮)，就是所谓sRGB，not gamma-corrected，是为了直接输出到显示器做的encoding
    // 所以shader里需要手动做一次 pow(1/2.2)
    FragColor = vec4(color, 1.0);
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));
}
