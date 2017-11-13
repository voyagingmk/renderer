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
uniform sampler2D ssao;
uniform sampler2D sssm;
uniform bool enableSSAO;


struct Light {
    int type;
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float far_plane;
    float constant;
    float Linear;
    float Quadratic;
    float cutOff;
    float outerCutOff;
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


void main()
{		
     // retrieve data from gbuffer
    vec3 FragPos = (viewInv * vec4(texture(gPosition, TexCoord).rgb, 1.0)).xyz;
    vec3 Normal = (viewInv * vec4(normalize(texture(gNormal, TexCoord).rgb), 0.0)).xyz;
    float AmbientOcclusion = texture(ssao, TexCoord).r;
    Material material;
    material.albedo = texture(gAlbedo, TexCoord).rgb;
    vec4 pbrData = texture(gPBR, TexCoord).rgba;
    material.metallic = pbrData.r;
    material.roughness = pbrData.g;
    material.specular = pbrData.b;
    material.ao = pbrData.a;

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
        Light light = lights[i];
        vec3 lightDir;
        int type = light.type;
        float intensity = 1.0;
        if (type == 1) { // directional light
            lightDir = -light.Direction;
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

        // add to outgoing radiance Lo
        Lo += (kD * material.albedo / PI + brdf) * radiance * NdotL * intensity;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    float shadow = texture(sssm, TexCoord).r;
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * material.albedo * material.ao;
    
    vec3 color = ambient + Lo * ( 1- shadow);

    if (enableSSAO) {
        color *= AmbientOcclusion;
    }

    // HDR tonemapping
    // color = color / (color + vec3(1.0));
    // gamma correct
    // 0.5 gamma-corrected, 就是线性空间的值
    // 0.5**(1/2.2) = 0.729（变亮)，就是所谓sRGB，not gamma-corrected，是为了直接输出到显示器做的encoding
    // 所以shader里需要手动做一次 pow(1/2.2)
    FragColor = vec4(color, 1.0);
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));
}
