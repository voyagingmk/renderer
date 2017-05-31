#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

// out vec3 ourColor;
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 viewAndProj;
uniform mat4 model;
uniform mat4 normalMat;

void main()
{
	mat4 mvp = model * viewAndProj;
	gl_Position = vec4(position, 1.0f) * mvp;
	FragPos = vec3(vec4(position, 1.0f) * model);
	Normal = normal * mat3(normalMat);
	// Normal = mat3(transpose(inverse(model))) * normal;
	// We swap the y-axis by substracing our coordinates from 1.
	TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}