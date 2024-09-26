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
uniform float boneScaleFactor;  // Agregamos este uniform para la escala

out vec3 FragPos;
out vec3 FragPosView; // Para SSAO
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

void main()
{
    if (useBones == false)
    {
        // Transformación normal sin animación de huesos
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
    else
    {
        // Inicialización de variables para acumulación de posiciones y normales
        vec4 totalPosition = vec4(0.0f);
        vec3 totalNormal = vec3(0.0f);
        vec3 totalTangent = vec3(0.0f);
        vec3 totalBitangent = vec3(0.0f);

        // Aplicar transformaciones de hueso con un escalado
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIds[i] == -1)
                continue;

            if (boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(aPos, 1.0f);
                break;
            }

            // Aquí aplicamos el factor de escala en las transformaciones de hueso
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            localPosition.xyz *= boneScaleFactor;  // Escalamos la posición por el factor
            totalPosition += localPosition * weights[i];

            // Transformaciones normales sin escala (las normales no se deben escalar)
            totalNormal += mat3(finalBonesMatrices[boneIds[i]]) * aNormal * weights[i];
            totalTangent += mat3(finalBonesMatrices[boneIds[i]]) * aTangent * weights[i];
            totalBitangent += mat3(finalBonesMatrices[boneIds[i]]) * aBitangent * weights[i];
        }

        // Aplicar la transformación del modelo
        vec4 worldPos = model * totalPosition;
        FragPos = worldPos.xyz;
        TexCoords = aTexCoords;

        // Proyectar a la vista
        vec4 viewPos = view * worldPos;
        FragPosView = viewPos.xyz; // Para SSAO

        // Aplicar las transformaciones a las normales, tangentes y bitangentes
        mat3 normalMatrix = transpose(inverse(mat3(model)));
        Normal = normalize(normalMatrix * totalNormal);
        Tangent = normalize(normalMatrix * totalTangent);
        Bitangent = normalize(normalMatrix * totalBitangent);

        // Posición final en el clip space
        gl_Position = projection * viewPos;
    }
}






//#version 460 core
//
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoords;
//layout (location = 2) in vec3 aNormal;
//layout (location = 3) in vec3 aTangent;
//layout (location = 4) in vec3 aBitangent;
//layout (location = 5) in ivec4 boneIds; 
//layout (location = 6) in vec4 weights;
//
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//
//const int MAX_BONES = 100;
//const int MAX_BONE_INFLUENCE = 4;
//
//uniform bool useBones;
//uniform mat4 finalBonesMatrices[MAX_BONES];
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
//    if (useBones == false)
//    {
//        vec4 worldPos = model * vec4(aPos, 1.0);
//        FragPos = worldPos.xyz; 
//        TexCoords = aTexCoords;
//
//        vec4 viewPos = view * worldPos;
//        FragPosView = viewPos.xyz; // Para SSAO
//
//        mat3 normalMatrix = transpose(inverse(mat3(model)));
//        Normal = normalize(normalMatrix * aNormal);
//
//        Tangent = normalize(normalMatrix * aTangent);
//        Bitangent = normalize(normalMatrix * aBitangent);
//
//        gl_Position = projection * viewPos;
//    }
//    else
//    {
//        vec4 totalPosition = vec4(0.0f);
//
//        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
//        {
//            if(boneIds[i] == -1) 
//                continue;
//            if(boneIds[i] >=MAX_BONES) 
//            {
//                totalPosition = vec4(aPos,1.0f);
//                break;
//            }
//            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
//            totalPosition += localPosition * weights[i];
//            vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
//        }
//
//        vec4 worldPos = model * vec4(aPos, 1.0);
//        FragPos = worldPos.xyz; 
//        TexCoords = aTexCoords;
//        mat3 normalMatrix = transpose(inverse(mat3(model)));
//        Normal = normalize(normalMatrix * aNormal);
//        Tangent = normalize(normalMatrix * aTangent);
//        Bitangent = normalize(normalMatrix * aBitangent);
//
//        mat4 viewModel = view * model;
//        gl_Position =  projection * viewModel * totalPosition;
//	    
//        
//    }
//}
