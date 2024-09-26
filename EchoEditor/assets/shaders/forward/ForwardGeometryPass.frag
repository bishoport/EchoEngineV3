#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D albedoTexture;

void main()
{    
    FragColor = texture(albedoTexture, TexCoords);
}

