#version 330 core

layout(location = 0)  in vec3 in_Position;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 boneIds; 
layout (location = 6) in vec4 weights;

uniform mat4 shadowMVP;  // Matriz para calcular las sombras
uniform mat4 model;      // Matriz de modelo

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform bool useBones;  // Flag para saber si se deben usar los huesos
uniform mat4 finalBonesMatrices[MAX_BONES];  // Matrices finales de los huesos
uniform float boneScaleFactor;  // Factor de escala de los huesos

void main() {
    vec4 finalPosition = vec4(in_Position, 1.0);  // Posición inicial sin huesos

    if (useBones) {
        finalPosition = vec4(0.0);  // Reiniciamos para la acumulación

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (boneIds[i] == -1)
                continue;

            if (boneIds[i] >= MAX_BONES)
                break;

            // Aplicamos la transformación del hueso y escalamos la posición
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(in_Position, 1.0f);
            localPosition.xyz *= boneScaleFactor;  // Escalamos la posición por el factor
            finalPosition += localPosition * weights[i];
        }
    }

    // Aquí aplicamos la transformación del shadowMVP
    gl_Position = shadowMVP * model * finalPosition;
}