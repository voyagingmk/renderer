#version 410 core

layout (location = 0) in vec3 position;
layout (location = 4) in mat4 aModelMat;

uniform mat4 PV;

uniform mat4 modelMat;
uniform bool instanced;

void main()
{
	mat4 model = instanced ? aModelMat * modelMat: modelMat;
	vec4 WorldPos = model * vec4(position, 1.0f);
	gl_Position = PV * WorldPos;
}