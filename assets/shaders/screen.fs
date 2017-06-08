#version 410 core

out vec4 color;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{ 
    // color = vec4(1.0, 0.0, 0.0, 1.0);
    color = texture(texture1, TexCoord);
}