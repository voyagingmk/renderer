#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 vcolor;
layout (location = 3) in vec2 texCoord;

#define SMAA_RT_METRICS vec4(1.0 / 800.0, 1.0 / 600.0, 800.0, 600.0)

out vec2 TexCoord;
out vec4 Offsets[3];



void main()
{
    gl_Position = vec4(position, 1.0); 
    TexCoord = texCoord;
    Offsets[0] = fma(SMAA_RT_METRICS.xyxy, vec4(-1.0, 0.0, 0.0, -1.0), texCoord.xyxy);
    Offsets[1] = fma(SMAA_RT_METRICS.xyxy, vec4( 1.0, 0.0, 0.0,  1.0), texCoord.xyxy);
    Offsets[2] = fma(SMAA_RT_METRICS.xyxy, vec4(-2.0, 0.0, 0.0, -2.0), texCoord.xyxy);
}  