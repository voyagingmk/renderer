#version 410 core

uniform vec4 ourColor;
out vec4 color;

void main()
{
    color = vec4(ourColor.rgb, 1.0f);
}