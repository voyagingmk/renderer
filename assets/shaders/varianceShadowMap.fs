#version 410 core

in vec4 v_position;
out vec2 outColor;

void main()
{             
    float depth = v_position.z / v_position.w;
    depth = depth * 0.5 + 0.5;// TO NDC [0, 1]

    float moment1 = depth;
    float moment2 = depth * depth;

    float dx = dFdx(depth);
    float dy = dFdy(depth);
    moment2 += 0.25 * (dx * dx + dy * dy);
    outColor = vec2(moment1, moment2);
}