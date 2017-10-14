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
float radius = 2;
float bias = 0.025;


uniform mat4 view;
uniform mat4 proj;

void main()
{    
    // tile noise texture over screen based on screen dimensions divided by noise size  
    vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0); 

    // get input for SSAO algorithm
   // vec3 fragPos = (view * vec4(texture(gPosition, TexCoord).xyz, 1.0)).xyz;
   // fragPos = vec3(0.0, 0.0, 0.2);
    vec3 fragPos = texture(gPosition, TexCoord).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoord).xyz);
// normal = normalize(mat3(view) * texture(gNormal, TexCoord).xyz);
   // normal = vec3(0.0, 0.0, 1.0);
   // vec3 normal = normalize(texture(gNormal, TexCoord).xyz);
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
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel samplePos
      //  sampleDepth =  0.5;
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }

    occlusion = 1.0 - (occlusion / kernelSize);  
    FragColor = occlusion;
}