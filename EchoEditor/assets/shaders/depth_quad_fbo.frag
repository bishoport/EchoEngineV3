#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthTexture;
uniform float near;
uniform float far;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Volver al espacio de coordenadas normalizadas del dispositivo (NDC)
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    float depthValue = texture(depthTexture, TexCoords).r;
    depthValue = LinearizeDepth(depthValue); // Convertir a profundidad lineal
    // Mapear a [0, 1] basado en near y far
    float depthVisual = (depthValue - near) / (far - near);
    FragColor = vec4(depthVisual, depthVisual, depthVisual, 1.0); // Escala de grises
}
