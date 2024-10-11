#version 460 core
out vec4 FragColor;

uniform vec3 objectIDColor;  // Color que representa el ID del objeto

void main()
{
    FragColor = vec4(objectIDColor, 1.0); // Renderizar el color como ID
}
