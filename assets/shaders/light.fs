#version 410 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

void main()
{    
  vec3 V = normalize(vec3(0, 0, 0) - FragPos);
  float diff = max(dot(normalize(Normal), V), 0.0);
  color = vec4(1.0, 1.0, 1.0, 1.0);
}
