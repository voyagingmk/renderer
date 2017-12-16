#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 4) in mat4 aModelMat;

uniform mat4 lightPV;
uniform mat4 modelMat;
uniform bool instanced;

out vec4 v_position;

void main()
{
	mat4 model = instanced ? aModelMat * modelMat: modelMat;
    gl_Position = lightPV * model * vec4(position, 1.0);
    v_position  = gl_Position;
}