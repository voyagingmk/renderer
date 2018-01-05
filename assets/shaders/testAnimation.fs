
#version 410 core

vec4 GetAmbient(vec3 _world_normal) {
    vec3 normal = normalize(_world_normal);
    vec3 alpha = (normal + 1.) * .5;
    vec4 bt = mix(
    vec4(.3, .3, .7, .7), vec4(.4, .4, .8, .8), alpha.xzxz);
    vec4 ambient = vec4(
        mix(vec3(bt.x, .3, bt.y), vec3(bt.z, .8, bt.w), alpha.y), 1.);
    return ambient;
}

in vec3 v_world_normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D albedoMap;

void main() {
    vec3 albedo = texture(albedoMap, TexCoord).rgb;
    vec4 ambient = GetAmbient(v_world_normal);
    FragColor = vec4(albedo, 1.0);
}