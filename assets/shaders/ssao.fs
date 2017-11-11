#version 410 core

out float FragColor;

in vec2 TexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec2 screenSize;
uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 1.0;
float bias = 0.025;


uniform mat4 view;
uniform mat4 proj;

vec3 sampleGBufferPos(vec2 coord) {
    // vec3 p = texture(gPosition, coord).xyz;
    vec4 p = view * vec4(texture(gPosition, coord).xyz, 1.0);
    return p.xyz;
}

vec3 sampleGBufferNormal(vec2 coord) {
    // vec3 p = texture(gNormal, coord).xyz;
    vec3 p = mat3(view) * texture(gNormal, coord).xyz;
    return p;
}


void main()
{    
    // tile noise texture over screen based on screen dimensions divided by noise size  
    vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0); 

    // get input for SSAO algorithm
    vec3 fragPos = sampleGBufferPos(TexCoord);
    // vec3 fragPos = texture(gPosition, TexCoord).xyz;
    vec3 normal = sampleGBufferNormal(TexCoord);
    vec3 randomVec = normalize(texture(texNoise, TexCoord * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the samplePos kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get samplePos position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project samplePos position (to samplePos texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = proj * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get samplePos depth
        float sampleDepth = sampleGBufferPos(offset.xy).z;

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }

    occlusion = 1.0 - (occlusion / kernelSize);  
    FragColor = occlusion;
}