#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gPBR;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

struct Material {
    float metallic;
    float roughness;
    float ao;
}; 

uniform Material material;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;

uniform bool hasNormalMap;

void main()
{    
    gPosition = FragPos;
    vec3 albedo = texture(albedoMap, TexCoord).rgb;
    vec3 normal = texture(normalMap, TexCoord).rgb;
    if(!hasNormalMap) {
        gNormal = Normal;
    } else {
        normal = normalize(normal * 2.0 - 1.0);   
        gNormal = normalize(TBN * normal); 
    }
    gAlbedo = vec4(albedo, 1.0);
    gPBR.r = material.metallic;
    gPBR.g = material.roughness;
    gPBR.b = material.ao;
}