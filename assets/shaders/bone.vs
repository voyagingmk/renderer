#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in mat4 joint;


mat4 GetWorldMatrix() {
  // Rebuilds bone properties.
  // Bone length is set to zero to disable leaf rendering.
  float is_bone = joint[3].w;
  vec3 bone_dir = vec3(joint[0].w, joint[1].w, joint[2].w) * is_bone;
  float bone_len = length(bone_dir);

  // Setup rendering world matrix.
  float dot = dot(joint[2].xyz, bone_dir);
  vec3 binormal = abs(dot) < .01 ? joint[2].xyz : joint[1].xyz;

  mat4 world_matrix;
  world_matrix[0] = vec4(bone_dir, 0.);
  world_matrix[1] = 
    vec4(bone_len * normalize(cross(binormal, bone_dir)), 0.);
  world_matrix[2] =
    vec4(bone_len * normalize(cross(bone_dir, world_matrix[1].xyz)),
  0.);
  world_matrix[3] = vec4(joint[3].xyz, 1.);
  return world_matrix;
}

uniform mat4 modelMat;
uniform mat4 PV;
out vec3 Normal;

void main() {
  mat4 world_matrix = GetWorldMatrix();
  vec4 vertex = vec4(position.xyz, 1.);
  gl_Position = PV * modelMat * world_matrix * vertex;
  mat3 cross_matrix = mat3(
    cross(world_matrix[1].xyz, world_matrix[2].xyz),
    cross(world_matrix[2].xyz, world_matrix[0].xyz),
    cross(world_matrix[0].xyz, world_matrix[1].xyz));
  float invdet = 1.0 / dot(cross_matrix[2], world_matrix[2].xyz);
  mat3 normal_matrix = cross_matrix * invdet;
  Normal = normal_matrix * normal;
}