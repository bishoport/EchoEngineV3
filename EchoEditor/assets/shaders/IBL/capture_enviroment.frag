#version 460 core
out vec4 FragColor;

in vec3 WorldPos;
in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D albedoTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// Propiedades del material
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main()
{
    vec3 albedo = texture(albedoTexture, TexCoord).rgb;

    // Componentes de la iluminación
    vec3 ambient = ambientStrength * lightColor;

    // Componente difusa
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - WorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Componente especular
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Color final
    vec3 result = (ambient + diffuse + specular) * albedo;

    // Para HDR, permitimos que los valores excedan 1.0
    FragColor = vec4(result, 1.0);
}
