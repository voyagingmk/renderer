#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 lightPV;
uniform mat4 model;

void main()
{
    mat4 mvp = lightPV * model;
    gl_Position = mvp * vec4(position, 1.0f);
}  