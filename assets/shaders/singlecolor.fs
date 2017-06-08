#version 410 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos; 
in vec2 TexCoord;
in vec3 Normal;
in vec3 VColor; 

out vec4 color;

uniform vec3 viewPos;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform Material material;
uniform Light light; 

void main()
{
    color = vec4(0.04, 0.28, 0.26, 1.0);
}