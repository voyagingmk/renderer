
#version 410 core

out vec4 FragColor;
in vec3 Normal;
  
void main()
{
    FragColor = vec4(Normal, 1.0);
} 