#version 410 core

out vec4 color;

in vec2 TexCoord;
uniform sampler2D texture1;

//uniform float near_plane;
//uniform float far_plane;
float near_plane = 0.1;
float far_plane = 10000.0;

float LinearizeDepth(float depth);

void main()
{ 
    // depth map
    float depthValue = texture(texture1, TexCoord).r;
    //color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    color = vec4(vec3(depthValue), 1.0); // orthographic
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}
