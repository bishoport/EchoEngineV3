#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 FragPosView; // Para SSAO
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;

    vec4 viewPos = view * worldPos;
    FragPosView = viewPos.xyz; // Para SSAO

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal);

    Tangent = normalize(normalMatrix * aTangent);
    Bitangent = normalize(normalMatrix * aBitangent);

    gl_Position = projection * viewPos;
}


//#version 460 core
//
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoords;
//layout (location = 2) in vec3 aNormal;
//layout (location = 3) in vec3 aTangent;
//layout (location = 4) in vec3 aBitangent;
//
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//
//out vec3 FragPos;
//out vec3 FragPosView; // Para SSAO
//out vec2 TexCoords;
//out vec3 Normal;
//out vec3 Tangent;
//out vec3 Bitangent;
//
//void main()
//{
//    vec4 worldPos = model * vec4(aPos, 1.0);
//    FragPos = worldPos.xyz; 
//    TexCoords = aTexCoords;
//
//    vec4 viewPos = view * worldPos;
//    FragPosView = viewPos.xyz; // Para SSAO
//
//    mat3 normalMatrix = transpose(inverse(mat3(model)));
//    Normal = normalize(normalMatrix * aNormal);
//
//    Tangent = normalize(normalMatrix * aTangent);
//    Bitangent = normalize(normalMatrix * aBitangent);
//
//    gl_Position = projection * viewPos;
//}