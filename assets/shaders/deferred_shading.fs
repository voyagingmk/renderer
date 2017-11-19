#version 410 core

out vec4 FragColor;

in vec2 TexCoord;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float specular;
    float ao;
}; 

uniform sampler2D gPosition; // FragPos
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPBR;
uniform samplerCube depthCubeMap;
uniform sampler2D depthMap;


struct Light {
    int type;
    float intensity;
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float far_plane;
    float constant;
    float Linear;
    float Quadratic;
    float cutOff;
    float outerCutOff;
    bool castShadow;
    mat4 lightPV;
};
uniform Light light;
uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 viewInv;
uniform float depthBias;
uniform float diskFactor;
uniform bool begin;


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

float ShadowCalculation_Soft(vec3 fragPos, Light light)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.Position;
    // Use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthCubeMap, fragToLight).r;
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
        closestDepth = texture(depthCubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
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
    float closestDepth = texture(depthCubeMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= light.far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = depthBias; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;        

    return shadow;
}


float ShadowCalculation_Dir(vec3 fragPos, Light light, vec3 N, bool pcf)
{
    vec4 fragPosLightSpace = light.lightPV * vec4(fragPos, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 lightDir = normalize(light.Position - fragPos);
    float bias = max(0.05 * (1.0 - dot(N, lightDir)), 0.005);
    bias = 0.005;
    float shadow = 0.0;
    // check whether current frag pos is in shadow
    // float 
    if (pcf) {
        // PCF
        vec2 texelSize = 1.0 / textureSize(depthMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;
    } else {
        shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    }
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
    // shadow = closestDepth;
    return shadow;
}


vec3 calRadiance(vec3 FragPos, Material material, Light light, vec3 F0, vec3 N, vec3 V) {
    vec3 lightDir;
    int type = light.type;
    float intensity = 1.0;
    if (type == 1) { // directional light
        lightDir = normalize(light.Position);
    } else if(type == 2) {
        lightDir = light.Position - FragPos;
    } else if(type == 3) {
        lightDir = light.Position - FragPos;
        float theta = dot(normalize(lightDir), -normalize(light.Direction)); 
        float epsilon = (light.cutOff - light.outerCutOff);
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    }
    // calculate per-light radiance
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);
    float distance = length(lightDir);
    float attenuation = 1.0 / ( 1.0 +light.Linear * distance +light.Quadratic * distance * distance);
    vec3 radiance = light.Color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, material.roughness);   
    float G   = GeometrySmith(N, V, L, material.roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 brdf = nominator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    kS = kS * material.specular;
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


    float shadow = 0;
    if (light.castShadow) {
        if (type == 1) {
            shadow = ShadowCalculation_Dir(FragPos, light, N, false);
            //return vec3(1 - shadow);
        } 
        else if (type == 2) {
            shadow = ShadowCalculation_Hard(FragPos, light);
        }
    }
    vec3 Lo = (kD * material.albedo / PI + brdf) * radiance * NdotL * light.intensity * intensity * (1.0 - shadow);  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    return Lo;
}

void main()
{		
     // retrieve data from gbuffer
    vec3 FragPos = (viewInv * vec4(texture(gPosition, TexCoord).rgb, 1.0)).xyz;
    vec3 Normal = (viewInv * vec4(normalize(texture(gNormal, TexCoord).rgb), 0.0)).xyz;
    Material material;
    material.albedo = texture(gAlbedo, TexCoord).rgb;
    vec4 pbrData = texture(gPBR, TexCoord).rgba;
    material.metallic = pbrData.r;
    material.roughness = pbrData.g;
    material.specular = pbrData.b;
    material.ao = pbrData.a;

    if (begin) {
        // ambient lighting (note that the next IBL tutorial will replace 
        // this ambient lighting with environment lighting).
        vec3 ambient = vec3(0.03) * material.albedo * material.ao;
        //FragColor = vec4(ambient, 1.0);
        return;
    }
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use their albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material.albedo, material.metallic);
    vec3 Lo = calRadiance(FragPos, material, light, F0, N, V);
    FragColor = vec4(Lo, 1.0);
}
