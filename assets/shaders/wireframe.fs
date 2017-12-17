#version 410 core

uniform vec3 wireColor;

out vec4 FragColor;

void main()
{    

    FragColor = vec4(wireColor, 1);
}