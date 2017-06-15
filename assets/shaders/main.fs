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
in vec4 FragPosLightSpace;

out vec4 color;

uniform vec3 viewPos;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform Material material;
uniform Light light; 

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	if (projCoords.z > 1.0) {
		return 0.0;
	}
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(texture2, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    
	// shadow acne
	//float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
	
	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(texture2, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(texture2, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

	// float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  	

    return shadow;
}


void main()
{
	vec4 objectColor = texture(texture1, TexCoord);
	vec3 normal = normalize(Normal);
	// dir: frag -> light
	vec3 lightDir = normalize(light.position - FragPos); 
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// dir:  light -> frag 
	vec3 reflectDir = reflect(-lightDir, normal);  
	// Phong
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	// Blinn-Phong
	// float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	vec3 ambient  = light.ambient * material.ambient;
	vec3 diffuse  = light.diffuse * (diff * material.diffuse);
	vec3 specular = light.specular * (spec * material.specular); 

	float distance    = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + 
    light.quadratic * (distance * distance));    
 	
	// calculate shadow
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
	bias = 0.0;
    float shadow = ShadowCalculation(FragPosLightSpace, bias);      

	color = vec4((ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation , 1.0) * objectColor;

	//float d = texture(texture2, TexCoord).r;
	//color = vec4(d,d,d, 1.0);
}