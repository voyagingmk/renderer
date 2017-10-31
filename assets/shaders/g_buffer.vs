#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

// out vec3 ourColor;
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out mat3 TBN;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 PV;

uniform mat4 model;
uniform mat4 normalMat;
uniform bool hasNormalMap;

void main()
{
	vec4 WorldPos = model * vec4(position, 1.0f);
	FragPos = (view * WorldPos).xyz;
	gl_Position = PV * WorldPos;
	// mat3 normalMatrix = mat3(view * transpose(inverse(model))); // Wrong!
	// mat3 normalMatrix = mat3(transpose(inverse(view * mat4(mat3(model)))));
	mat3 normalMatrix = mat3(view * mat4(transpose(inverse(mat3(model)))));   
	Normal = normalMatrix * normal;
	if (hasNormalMap) {
		TBN = mat3(tangent, bitangent, normal);
		TBN = normalMatrix * TBN; 
	}
	// Normal = mat3(transpose(inverse(model))) * normal;
	// We swap the y-axis by substracing our coordinates from 1.
	TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}