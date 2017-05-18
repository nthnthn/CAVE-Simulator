#version 330 core
in vec3 TexCoords;
layout (location = 0) out vec3 color;

uniform samplerCube skybox;

void main()
{    
    color = vec3(texture(skybox, TexCoords));
}