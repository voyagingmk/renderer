#version 410 core

out vec4 color;

in vec2 TexCoord;

uniform sampler2D texture1;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main2()
{
    vec2 texCorrd = TexCoord;
    texCorrd.y = 1.0 - texCorrd.y;
    vec3 c = texture(texture1, texCorrd.xy).rgb;
    float sigDist = median(c.r, c.g, c.b) - 0.5;
    float opacity = clamp(sigDist/fwidth(sigDist) + 0.5, 0.0, 1.0);
    color = vec4(1.0, 1.0, 1.0, opacity);
}

void main()
{
    vec2 texCorrd = TexCoord;
    texCorrd.y = 1.0 - texCorrd.y;
    float dist = texture(texture1, texCorrd.xy).r;
    float smoothing = fwidth(dist) * 0.0001;
	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, dist);
    color = vec4(1.0, 1.0, 1.0, alpha);
}