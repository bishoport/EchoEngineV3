#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 boneIds; 
layout (location = 6) in vec4 weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform bool useBones;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 FragPos;
out vec3 FragPosView; // Para SSAO
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

void main()
{
    vec4 worldPos;
    
    if (useBones == false)
    {
        worldPos = model * vec4(aPos, 1.0);
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
    else
    {
        vec4 totalPosition = vec4(0.0f);
        vec3 totalNormal = vec3(0.0f);

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIds[i] == -1) 
                continue;

            if (boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(aPos, 1.0f);
                break;
            }

            // Aplicar las matrices de huesos
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];

            // Aplicar la transformación de huesos a la normal
            mat3 boneTransform = mat3(finalBonesMatrices[boneIds[i]]);
            vec3 localNormal = boneTransform * aNormal;
            totalNormal += localNormal * weights[i];
        }

        // Transformación final en espacio mundial
        worldPos = model * totalPosition;
        FragPos = worldPos.xyz;
        TexCoords = aTexCoords;

        // Transformar las normales con la matriz del modelo
        mat3 normalMatrix = transpose(inverse(mat3(model)));
        Normal = normalize(normalMatrix * totalNormal);  // Normal transformada por huesos

        // Transforma las tangentes y bitangentes de manera similar si las necesitas
        Tangent = normalize(normalMatrix * aTangent);
        Bitangent = normalize(normalMatrix * aBitangent);

        // Obtener posición en espacio de vista
        vec4 viewPos = view * worldPos;
        FragPosView = viewPos.xyz; // Para SSAO

        // Posición final del vértice
        gl_Position = projection * viewPos;
    }

}
