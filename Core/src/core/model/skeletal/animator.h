#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "animation.h"
#include "bone.h"

namespace libCore
{
    class Animator
    {
    public:
        Animator() = default;  // Constructor por defecto

        // Método para asignar una animación en tiempo de ejecución
        void SetAnimation(Ref<Animation> animation)
        {
            m_CurrentAnimation = animation;
            if (!animation) {
                std::cerr << "Error: No se puede asignar una animación nula." << std::endl;
                return;
            }

            m_CurrentAnimation = animation;
            m_CurrentTime = 0.0f;  // Reinicia el tiempo de la animación
        }

        void UpdateAnimation(float dt)
        {
            if (!m_CurrentAnimation)
            {
                return;
            }

            m_DeltaTime = dt;
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }

        /*void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
        {
            if (!m_CurrentAnimation)
            {
                return;
            }

            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            Bone* bone = m_CurrentAnimation->FindBone(nodeName);

            if (bone)
            {
                bone->Update(m_CurrentTime);
                nodeTransform = bone->GetLocalTransform();
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
            {
                CalculateBoneTransform(&node->children[i], globalTransformation);
            }
        }*/

        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
        {
            if (!m_CurrentAnimation)
            {
                return;
            }

            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            // Buscar si este nodo tiene un hueso correspondiente en la animación
            Bone* bone = m_CurrentAnimation->FindBone(nodeName);

            if (bone)
            {
                // Actualizar la transformación del hueso en base al tiempo de la animación
                bone->Update(m_CurrentTime);
                nodeTransform = bone->GetLocalTransform();  // Esta es la transformación local del hueso en este frame
            }

            // La transformación global del hueso se obtiene multiplicando la transformación del padre con la transformación local
            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            // Obtener el BoneInfo del mapa de huesos
            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;

                // Aplicar la matriz global y la matriz de offset del hueso
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            // Recursivamente calcular las transformaciones para los hijos de este nodo
            for (int i = 0; i < node->childrenCount; i++)
            {
                CalculateBoneTransform(&node->children[i], globalTransformation);  // Propagar la transformación global hacia los hijos
            }
        }


        std::vector<glm::mat4> GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }

        // Nuevo método HasAnimation para comprobar si hay una animación activa
        bool HasAnimation() const {
            return m_CurrentAnimation != nullptr;
        }

        float GetCurrentTime() const { return m_CurrentTime; }
        float GetDeltaTime() const { return m_DeltaTime; }

    private:
        std::vector<glm::mat4> m_FinalBoneMatrices = std::vector<glm::mat4>(100, glm::mat4(1.0f));  // Inicializa 100 matrices
        Ref<Animation> m_CurrentAnimation = nullptr;
        float m_CurrentTime = 0.0f;
        float m_DeltaTime = 0.0f;
    };
}

