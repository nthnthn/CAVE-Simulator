#version 330 core
in vec2 TexCoords;
out vec4 color;
uniform sampler2D texFramebuffer;

void main()
{
    color = vec4(TexCoords, 0.0f, 1.0f);
}