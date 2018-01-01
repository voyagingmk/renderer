#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;

uniform mat4 modelMat;

mat4 GetWorldMatrix() {
    return modelMat;
}

uniform mat4 PV;

out vec3 v_world_normal;

void main() {
    mat4 world_matrix = GetWorldMatrix();
    vec4 vertex = vec4(position.xyz, 1.);
    gl_Position = PV * world_matrix * vertex;
    mat3 cross_matrix = mat3(
    cross(world_matrix[1].xyz, world_matrix[2].xyz),
    cross(world_matrix[2].xyz, world_matrix[0].xyz),
    cross(world_matrix[0].xyz, world_matrix[1].xyz));
    float invdet = 1.0 / dot(cross_matrix[2], world_matrix[2].xyz);
    mat3 normal_matrix = cross_matrix * invdet;
    v_world_normal = normal_matrix * normal;
}