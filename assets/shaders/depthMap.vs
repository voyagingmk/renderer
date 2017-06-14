#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 vcolor;
layout (location = 3) in vec2 texCoord;

uniform mat4 PV;
uniform mat4 model;

void main()
{
    mat4 mvp = PV * model;
    gl_Position = mvp * vec4(position, 1.0f);
}  