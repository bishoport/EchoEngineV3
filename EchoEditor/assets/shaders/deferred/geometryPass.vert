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
out vec3 FragPosView;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

void main()
{
    vec4 worldPos;
    
    if (useBones == false)
    {
        // Apply scaling for debugging
        vec4 scaledModelPos = model * vec4(aPos, 1.0) * 100.0;  
        worldPos = scaledModelPos;
        
        FragPos = worldPos.xyz; 
        TexCoords = aTexCoords;

        vec4 viewPos = view * worldPos;
        FragPosView = viewPos.xyz;

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

            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];

            mat3 boneTransform = mat3(finalBonesMatrices[boneIds[i]]);
            vec3 localNormal = boneTransform * aNormal;
            totalNormal += localNormal * weights[i];
        }

        // Apply scaling for debugging
        vec4 scaledModelPos = model * totalPosition * 100.0;
        worldPos = scaledModelPos;
        
        FragPos = worldPos.xyz;
        TexCoords = aTexCoords;

        mat3 normalMatrix = transpose(inverse(mat3(model)));
        Normal = normalize(normalMatrix * totalNormal);

        Tangent = normalize(normalMatrix * aTangent);
        Bitangent = normalize(normalMatrix * aBitangent);

        vec4 viewPos = view * worldPos;
        FragPosView = viewPos.xyz;

        gl_Position = projection * viewPos;
    }
}
