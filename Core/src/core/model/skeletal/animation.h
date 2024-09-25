#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "bone.h"
#include <functional>
#include "animdata.h"
#include "../Model.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace libCore
{
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
    public:
        Animation() = default;

        Animation(const std::string& animationPath, Ref<Model> unifiedModel)
        {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
            assert(scene && scene->mRootNode);
            auto animation = scene->mAnimations[0];
            m_Duration = animation->mDuration;
            m_TicksPerSecond = animation->mTicksPerSecond;
            aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
            globalTransformation = globalTransformation.Inverse();
            ReadHierarchyData(m_RootNode, scene->mRootNode);
            ReadMissingBones(animation, unifiedModel);

            // Inicializamos las matrices de los huesos
            m_FinalBoneMatrices.resize(100, glm::mat4(1.0f));  // 100 como número máximo de huesos, puedes ajustarlo si lo necesitas
        }

        ~Animation() {}

        Bone* FindBone(const std::string& name)
        {
            auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                [&](const Bone& Bone)
                {
                    return Bone.GetBoneName() == name;
                }
            );
            if (iter == m_Bones.end()) return nullptr;
            else return &(*iter);
        }

        // Función para actualizar la transformación de los huesos
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
        {
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            Bone* bone = FindBone(nodeName);

            if (bone)
            {
                bone->Update(m_CurrentTime);
                nodeTransform = bone->GetLocalTransform();
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto boneInfoMap = GetBoneIDMap();
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
        }

        // Función que retorna las matrices finales de los huesos
        const std::vector<glm::mat4>& GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }

        inline float GetTicksPerSecond() { return m_TicksPerSecond; }
        inline float GetDuration() { return m_Duration; }
        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
        inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }

        // Actualizar la animación
        void Update(float deltaTime)
        {
            m_CurrentTime += m_TicksPerSecond * deltaTime;
            if (m_CurrentTime > m_Duration) {
                m_CurrentTime = fmod(m_CurrentTime, m_Duration);
            }

            CalculateBoneTransform(&m_RootNode, glm::mat4(1.0f));
        }

    private:
        void ReadMissingBones(const aiAnimation* animation, Ref<Model> unifiedModel) {
            auto& boneInfoMap = unifiedModel->GetBoneInfoMap();
            int& boneCount = unifiedModel->GetBoneCount();

            int size = animation->mNumChannels;
            for (int i = 0; i < size; ++i) {
                auto channel = animation->mChannels[i];
                std::string boneName = channel->mNodeName.data;

                if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                    BoneInfo newBoneInfo;
                    newBoneInfo.id = boneCount;
                    boneInfoMap[boneName] = newBoneInfo;
                    boneCount++;
                }

                m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[boneName].id, channel));
            }

            m_BoneInfoMap = boneInfoMap;
        }

        void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
            assert(src);

            dest.name = src->mName.data;
            dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
            dest.childrenCount = src->mNumChildren;

            for (int i = 0; i < src->mNumChildren; i++) {
                AssimpNodeData newData;
                ReadHierarchyData(newData, src->mChildren[i]);
                dest.children.push_back(newData);
            }
        }

        float m_Duration;
        float m_CurrentTime = 0.0f;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;

        std::vector<glm::mat4> m_FinalBoneMatrices;  // Matrices finales de los huesos
    };
}
