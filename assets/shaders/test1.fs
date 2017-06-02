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

out vec4 color;

uniform vec3 viewPos;
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;
uniform Material material;
uniform Light light; 


void main()
{
	vec4 objectColor = texture(ourTexture1, TexCoord);
	color = objectColor;
	vec3 norm = normalize(Normal);
	// dir: frag -> light
	vec3 lightDir = normalize(light.position - FragPos); 
	float diff = max(dot(norm, lightDir), 0.0);

	vec3 viewDir = normalize(viewPos - FragPos);
	// dir:  light -> frag 
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	vec3 ambient  = light.ambient * material.ambient;
	vec3 diffuse  = light.diffuse * (diff * material.diffuse);
	vec3 specular = light.specular * (spec * material.specular); 

	float distance    = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
    light.quadratic * (distance * distance));    

	color = vec4((ambient + diffuse + specular) * attenuation, 1.0) * objectColor;
}