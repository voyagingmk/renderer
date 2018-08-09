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
        // 0.5**(1/2.2) = 0.729（变亮)，就是所谓sRGB，not gamma-corrected
        // 这是为了直接输出到显示器做的encoding
        // 所以shader里需要手动做一次 pow(1/2.2)
        // 再补充一个例子：
        // 已知人眼的(美术)中灰反射率是0.218，做个gamma编码（校正）：
        // Math.pow(0.218, 1/2.2) = 0.5003782295852497
        // 变成了0.5，相当于把灰阶编码到更大的范围，例如这样理解：[0,0.218]映射到[0, 0.5]
        // 这样编码处理后用8bits来存储，0.5变成128，就相当于尽可能保留了灰阶颜色（人眼更为敏感的颜色）
        // 当然保存下来的图片是偏亮的，不过因为是文件状态，并没有问题
        // 把这样编码的颜色输出到显示器，显示器做了个gamma解码：
        // Math.pow(128/256, 2.2) = 0.218 就知道了原来的颜色值
        // 然后嘉禾显示器能支持[0,n]种颜色，那么就0.218 * n 得到对应的颜色
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