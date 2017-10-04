#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gPBR;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

struct Material {
    float metallic;
    float roughness;
    float ao;
}; 

uniform Material material;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedo.rgba = texture(texture1, TexCoord);
    gPBR.r = material.metallic;
    gPBR.g = material.roughness;
    gPBR.b = material.ao;
    // gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    // gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}