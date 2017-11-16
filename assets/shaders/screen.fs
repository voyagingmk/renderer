#version 410 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform bool noGamma;
uniform bool noToneMapping;

vec4 Inversion();
vec4 Grayscale();
vec4 Sharpen();
vec4 Blur();
vec4 EdgeDetection();
vec3 ACESToneMapping(vec3 color, float adapted_lum);

void main()
{ 
    vec3 c = vec3(texture(texture1, TexCoord).rgb);
    vec4 color = vec4(c, 1.0);

    // reinhard tone mapping
    //vec3 mapped = color.rgb / (color.rgb + vec3(1.0));

    if (!noToneMapping) {
       color.rgb = ACESToneMapping(color.rgb, 1.0);
    }

    if (!noGamma) {   
        // gamma correct
        // 0.5 gamma-corrected, 就是线性空间的值
        // 0.5**(1/2.2) = 0.729（变亮)，就是所谓sRGB，not gamma-corrected，是为了直接输出到显示器做的encoding
        // 所以shader里需要手动做一次 pow(1/2.2)
        color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
    }
    
    FragColor = color;
    // color = Inversion();
    // color = Grayscale();
    // color = Sharpen();
    // color = Blur();
    // color = EdgeDetection();
}


vec3 ACESToneMapping(vec3 color, float adapted_lum) {
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
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