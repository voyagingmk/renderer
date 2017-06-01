#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

// out vec3 ourColor;
out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out vec2 debug;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 PV;
uniform mat4 model;
uniform mat4 normalMat;

void main()
{
	if(PV != proj * view) {
		debug = vec2(0.0,0.0);
	} else {
		debug = vec2(1.0, 1.0);
	}
	mat4 mvp = PV * model;
	// mat4 mvp =  proj * view * model;
	gl_Position = mvp * vec4(position, 1.0f);
	FragPos = vec3(model * vec4(position, 1.0f));
	Normal = normal * mat3(normalMat);
	// Normal = mat3(transpose(inverse(model))) * normal;
	// We swap the y-axis by substracing our coordinates from 1.
	TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}