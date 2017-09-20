#version 410 core

in vec3 TexCoord;
out vec4 color;

uniform samplerCube skybox;

void main()
{    
    color = texture(skybox, TexCoord);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
   // color = vec4(1.0, 0.0, 0.0, 0.0);
}