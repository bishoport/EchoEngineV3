#version 460 core

in vec2 TexCoord;
in vec3 Normal;  
in vec3 FragPos;


uniform sampler2D albedoTexture;
uniform vec3 albedoColor;
uniform sampler2D normalTexture;
uniform sampler2D texture_specular1;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;

 

out vec4 FragColor;


void main()
{
    FragColor = texture(albedoTexture, TexCoord);
}


