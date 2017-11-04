#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gPBR;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

struct Material {
    float metallic;
    float roughness;
    float specular;
    float ao;
}; 

uniform Material material;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D maskMap;


uniform bool hasNormalMap;
uniform bool hasMaskMap;
uniform bool hasSpecularMap;


void main()
{    
    gPosition = FragPos;
    vec2 texcoord = vec2(TexCoord.x, TexCoord.y);
    if (hasMaskMap) {
        float alpha = texture(maskMap, texcoord).r;
        if(alpha <= 0.01) {
            discard;
            return;
        }
    }
    vec3 albedo = texture(albedoMap, texcoord).rgb;
    if(!hasNormalMap) {
        gNormal = Normal;
    } else {
        vec3 normal = texture(normalMap, texcoord).rgb;
        normal = normalize(normal * 2.0 - 1.0);   
        gNormal = normalize(TBN * normal); 
    }
    if(!hasSpecularMap) {
        material.specular = 1;
    } else {
        float specular = texture(specularMap, texcoord).r;
        material.specular = specular;
    }  
    gAlbedo = vec4(albedo, 1.0);
    gPBR.r = material.metallic;
    gPBR.g = material.roughness;
    gPBR.b = material.specular;
    gPBR.a = material.ao;
}