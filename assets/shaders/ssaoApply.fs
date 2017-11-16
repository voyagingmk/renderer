#version 410 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D img;
uniform sampler2D ssao;

void main()
{		
    vec3 color = texture(img, TexCoord).rgb;
    float AmbientOcclusion = texture(ssao, TexCoord).r;
    color *= AmbientOcclusion;
    FragColor = vec4(color, 1.0);
}