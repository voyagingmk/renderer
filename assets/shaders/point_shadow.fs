#version 410 core

//#define WITH_NORMALMAP_UNSIGNED 1
//#define WITH_NORMALMAP_GREEN_UP 1

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
uniform sampler2D texture1;
uniform samplerCube depthMap;
uniform sampler2D normTex;
uniform Material material;
uniform Light light; 
uniform float far_plane;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);


float ShadowCalculation(vec3 fragPos)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.position;
    // Use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // It is currently in linear range between [0,1]. Let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // Undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // Display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    // return shadow;
    return shadow;
}

void main()
{  
    /*
    vec3 fragToLight = FragPos - light.position;
    float closestDepth = texture(depthMap, fragToLight).r;
    color = vec4(vec3(closestDepth), 1.0);   
    return;
    */
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
    float shadow = ShadowCalculation(FragPos);      

	color = vec4((ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation , 1.0) * objectColor;

	//float d = texture(depthMap, TexCoord).r;
	//color = vec4(d,d,d, 1.0);
}