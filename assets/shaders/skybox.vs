#version 410 core

layout (location = 0) in vec3 position;
out vec3 TexCoord;


uniform mat4 proj;
uniform mat4 view;
uniform mat4 PV;
uniform mat4 modelMat;

void main()
{
    // TODO 在c++做mat4(mat3(view))会性能更好
    vec4 pos = proj * mat4(mat3(view)) * vec4(position, 1.0f);
    gl_Position = pos.xyww;// 这一句保证了skybox的深度为1.0，即最深
    TexCoord = position;
}  