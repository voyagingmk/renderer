#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;

uniform float normalOffset;
uniform vec3 lightPos;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalMatrix * normalize(normal);
    vec3 L = normalize(lightPos - position);
    float cosAngle = clamp(1.0f - dot(L, N), 0.0f, 1.0f);
    gl_Position = model * vec4(position, 1.0f) + vec4(N * normalOffset * cosAngle, 0.0f);
}  