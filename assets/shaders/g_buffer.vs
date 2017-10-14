#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 vcolor;
layout (location = 3) in vec2 texCoord;

// out vec3 ourColor;
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out vec3 VColor;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 PV;

uniform mat4 model;
uniform mat4 normalMat;

void main()
{
	vec4 WorldPos = model * vec4(position, 1.0f);
	// FragPos = (view * WorldPos).xyz;
	FragPos = WorldPos.xyz;
	gl_Position = PV * WorldPos;
	// ((V M)^-1)^T = (V^-1 M^-1)^T = (M^-1)^T (V^-1)^T
	mat3 normalMatrix = mat3(view * mat4(transpose(inverse(mat3(model)))));
	Normal = normalMatrix * normal;
	// Normal = mat3(transpose(inverse(model))) * normal;
	// We swap the y-axis by substracing our coordinates from 1.
	TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
	VColor = vcolor;
}