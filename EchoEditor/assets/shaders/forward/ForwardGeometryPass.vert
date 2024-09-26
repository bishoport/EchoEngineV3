#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 boneIds; 
layout (location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool useBones;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;

void main()
{
    // Inicialización de la posición
    vec4 totalPosition = vec4(0.0f);

    if (useBones) 
    {
        // Usar huesos para calcular la posición
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) 
        {
            if (boneIds[i] == -1) 
                continue;
            if (boneIds[i] >= MAX_BONES) 
            {
                totalPosition = vec4(aPos, 1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];
        }
    }
    else 
    {
        // Renderizar sin huesos, usando solo la posición original
        totalPosition = vec4(aPos, 1.0f);
    }

    // Aplicar la transformación de vista y proyección
    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;
    TexCoords = aTexCoords;
}
