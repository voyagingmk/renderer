#version 410 core

in vec3 FragPos; 
in vec2 TexCoord;
in vec3 Normal;
in vec3 VColor; 


out vec4 color;

void main()
{
    color = vec4(1.0,1.0,0.0, 1.0);
}
