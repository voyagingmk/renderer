
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
	mat4 mvp = PV * model;
	// mat4 mvp =  proj * view * model;
	gl_Position = mvp * vec4(position, 1.0f);
	FragPos = vec3(model * vec4(position, 1.0f));
	Normal =  mat3(normalMat) * normal;
	// Normal = mat3(transpose(inverse(model))) * normal;
	// We swap the y-axis by substracing our coordinates from 1.
	TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
	VColor = vcolor;
}