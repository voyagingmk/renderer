#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gPBR;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
}; 

uniform Material material;

void main()
{    
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo = vec4(material.albedo, 1.0);
    gPBR.r = material.metallic;
    gPBR.g = material.roughness;
    gPBR.b = material.ao;
}