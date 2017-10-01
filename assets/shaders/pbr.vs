#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 vcolor;
layout (location = 3) in vec2 texCoord;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 normalMat;
uniform mat4 model;
uniform mat4 PV;

void main()
{
    TexCoord = texCoord;
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(model) * normal;

    gl_Position =  PV * vec4(FragPos, 1.0);
}