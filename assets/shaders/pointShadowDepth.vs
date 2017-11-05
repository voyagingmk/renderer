#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;

uniform float normalOffset;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));   
    vec3 N = normalMatrix * normalize(normal);
    gl_Position = model * vec4(position, 1.0f) + vec4(N * normalOffset, 0.0f);
}  