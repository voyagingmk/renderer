#version 410 core


in vec3 FragPos; 
in vec2 TexCoord;
in vec3 Normal; 

out vec4 color;

uniform vec3 lightPos; 
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;


void main()
{
	vec3 ambientColor = vec3(1.0, 1.0, 1.0);
	vec3 lightColor = vec3(1.0, 1.0, 0.0);
	// Linearly interpolate between both textures (second texture is only slightly combined)
	//vec4 objectColor = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, TexCoord), 0.2);	
	vec4 objectColor = texture(ourTexture1, TexCoord);	
	float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * ambientColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor; 

	color = vec4(ambient + 2.0 * diffuse, 1.0) * objectColor;
}