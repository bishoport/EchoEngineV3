#version 460 core

out vec4 FragColor;

uniform vec3 axisColor;

void main()
{
    FragColor = vec4(axisColor, 1.0);
}
