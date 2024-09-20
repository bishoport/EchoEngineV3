#version 460 core

out vec4 FragColor;

uniform vec3 gridColor;
uniform float gridTransparency;

void main()
{
    // Usamos el valor de transparencia para el color de salida
    FragColor = vec4(gridColor, gridTransparency); // Color uniforme con transparencia
}


