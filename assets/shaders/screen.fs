#version 410 core

out vec4 color;

in vec2 TexCoord;

uniform sampler2D texture1;

vec4 Inversion();
vec4 Grayscale();
vec4 Sharpen();
vec4 Blur();
vec4 EdgeDetection();

void main()
{ 
    vec3 c = vec3(texture(texture1, TexCoord).rgb);
    color = vec4(c, 1.0);
     // reinhard tone mapping
    //vec3 mapped = color.rgb / (color.rgb + vec3(1.0));
     
    // gamma correction
    //color.rgb = pow(mapped, vec3(1.0 / 2.2));
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
    // color = Inversion();
    // color = Grayscale();
    // color = Sharpen();
    // color = Blur();
    // color = EdgeDetection();
}


// post-effect

// 反色
vec4 Inversion() {
    vec4 color = texture(texture1, TexCoord);
    return vec4(vec3(1.0 - color), 1.0);
}

vec4 Grayscale() {
    vec4 color = texture(texture1, TexCoord);
    float average = (color.r + color.g + color.b) / 3.0;
    return vec4(average, average, average, 1.0);
}

vec4 KernelCommon(float kernel[9]) {
    const float offset = 1.0 / 300.0;  
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(texture1, TexCoord.st + offsets[i]));
    }
    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += sampleTex[i] * kernel[i];
    
    return vec4(color, 1.0);
}  

vec4 Sharpen() {
    float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    return KernelCommon(kernel);
}

vec4 Blur() {
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16  
    );
    return KernelCommon(kernel);
}

vec4 EdgeDetection() {
    float kernel[9] = float[](
        1, 1, 1,
        1,-9, 1,
        1, 1, 1
    );
    return KernelCommon(kernel);
}