#version 460 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 FragPosView; // Para SSAO
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

uniform sampler2D albedoTexture;
uniform vec3 albedoColor;
uniform sampler2D normalTexture;
uniform sampler2D texture_specular1;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;

uniform float normalStrength; 
uniform float metallicValue;
uniform float roughnessValue;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec4 gMetallicRoughness; // Cambiado a vec4
layout (location = 4) out float gSpecular;
layout (location = 5) out vec3 gPositionView; // Para SSAO

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    gPositionView = FragPosView; // Para SSAO

    // Retrieve the normal from the normal map
    vec3 normal = texture(normalTexture, TexCoords).rgb;
    
    // Transform the normal from [0,1] range to [-1,1] range
    normal = normal * 2.0 - 1.0;

    // Apply the normal strength
    normal = normalize(mix(vec3(0.0, 0.0, 1.0), normal, normalStrength));

    // Compute TBN matrix
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    vec3 N = normalize(Normal);
    mat3 TBN = mat3(T, B, N);
    
    // Transform normal from tangent space to world space
    vec3 perturbedNormal = normalize(TBN * normal);
    
    // Store the per-fragment normals into the gbuffer
    gNormal = perturbedNormal;
    
    // Store the diffuse per-fragment color, multiplied by the uniform color
    gAlbedo = texture(albedoTexture, TexCoords).rgb * albedoColor;
    
    // Store specular intensity in gSpecular
    gSpecular = texture(texture_specular1, TexCoords).r;

    // Retrieve PBR maps and values
    float metallic = texture(metallicTexture, TexCoords).r * metallicValue;
    float roughness = texture(roughnessTexture, TexCoords).r * roughnessValue;

    // Store the metallic and roughness values
    gMetallicRoughness = vec4(metallic, roughness, 0.0, 1.0); // Third component can be used for another purpose or kept as zero
}



//#version 460 core
//
//in vec2 TexCoords;
//in vec3 FragPos;
//in vec3 FragPosView; // Para SSAO
//in vec3 Normal;
//in vec3 Tangent;
//in vec3 Bitangent;
//
//uniform sampler2D albedoTexture;
//uniform vec3 albedoColor;
//uniform sampler2D normalTexture;
//uniform sampler2D texture_specular1;
//uniform sampler2D metallicTexture;
//uniform sampler2D roughnessTexture;
//
//uniform float normalStrength; 
//uniform float metallicValue;
//uniform float roughnessValue;
//
//layout (location = 0) out vec3 gPosition;
//layout (location = 1) out vec3 gNormal;
//layout (location = 2) out vec3 gAlbedo;
//layout (location = 3) out vec3 gMetallicRoughness;
//layout (location = 4) out float gSpecular;
//layout (location = 5) out vec3 gPositionView; // Para SSAO
//
//void main()
//{    
//    // Store the fragment position vector in the first gbuffer texture
//    gPosition = FragPos;
//    gPositionView = FragPosView; // Para SSAO
//
//    // Retrieve the normal from the normal map
//    vec3 normal = texture(normalTexture, TexCoords).rgb;
//    
//    // Transform the normal from [0,1] range to [-1,1] range
//    normal = normal * 2.0 - 1.0;
//
//    // Apply the normal strength
//    normal = normalize(mix(vec3(0.0, 0.0, 1.0), normal, normalStrength));
//
//    // Compute TBN matrix
//    vec3 T = normalize(Tangent);
//    vec3 B = normalize(Bitangent);
//    vec3 N = normalize(Normal);
//    mat3 TBN = mat3(T, B, N);
//    
//    // Transform normal from tangent space to world space
//    vec3 perturbedNormal = normalize(TBN * normal);
//    
//    // Store the per-fragment normals into the gbuffer
//    gNormal = perturbedNormal;
//    
//    // Store the diffuse per-fragment color, multiplied by the uniform color
//    gAlbedo = texture(albedoTexture, TexCoords).rgb * albedoColor;
//    
//    // Store specular intensity in gSpecular
//    gSpecular = texture(texture_specular1, TexCoords).r;
//
//    // Retrieve PBR maps and values
//    float metallic = texture(metallicTexture, TexCoords).r * metallicValue;
//    float roughness = texture(roughnessTexture, TexCoords).r * roughnessValue;
//
//    // Store the metallic and roughness values
//    gMetallicRoughness = vec3(metallic, roughness, 0.0); // Third component can be used for another purpose or kept as zero
//}