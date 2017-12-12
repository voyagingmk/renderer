#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec2 TexCoord;
uniform mat4 PV;
uniform mat4 modelMat;

void main()
{
    gl_Position = PV * modelMat * vec4(position, 1.0); 
    TexCoord = texCoord;
}
