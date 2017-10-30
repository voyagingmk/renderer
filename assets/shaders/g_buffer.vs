#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

// out vec3 ourColor;
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 PV;

uniform mat4 model;
uniform mat4 normalMat;

void main()
{
	vec4 WorldPos = model * vec4(position, 1.0f);
	FragPos = (view * WorldPos).xyz;
	gl_Position = PV * WorldPos;
	// mat3 normalMatrix = mat3(view * transpose(inverse(model))); // Wrong!
	// mat3 normalMatrix = mat3(transpose(inverse(view * mat4(mat3(model)))));
	mat3 normalMatrix = mat3(view * mat4(transpose(inverse(mat3(model)))));
	Normal = normalMatrix * normal;
	// Normal = mat3(transpose(inverse(model))) * normal;
	// We swap the y-axis by substracing our coordinates from 1.
	TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}