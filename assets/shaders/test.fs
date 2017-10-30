#version 410 core

in vec3 FragPos; 
in vec2 TexCoord;
in vec3 Normal;


out vec4 color;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	color = texture(texture1, TexCoord);
  //  color = vec4(1.0,1.0,0.0, 1.0);
}
