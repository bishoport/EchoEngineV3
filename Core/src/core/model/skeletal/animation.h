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
		}

		~Animation()
		{
		}

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


		inline float GetTicksPerSecond() { return m_TicksPerSecond; }
		inline float GetDuration() { return m_Duration; }
		inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
		inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}

	private:
		void ReadMissingBones(const aiAnimation* animation, Ref<Model> unifiedModel) {
			auto& boneInfoMap = unifiedModel->GetBoneInfoMap();
			int& boneCount = unifiedModel->GetBoneCount();

			// Leer los huesos desde el archivo de animación.
			int size = animation->mNumChannels;
			for (int i = 0; i < size; ++i) {
				auto channel = animation->mChannels[i];
				std::string boneName = channel->mNodeName.data;

				// Agregar los huesos que faltan al mapa unificado.
				if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
					BoneInfo newBoneInfo;
					newBoneInfo.id = boneCount;
					boneInfoMap[boneName] = newBoneInfo;
					boneCount++;
				}

				// Añadir el hueso a la animación.
				m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[boneName].id, channel));
			}

			// Asignar el mapa unificado a `m_BoneInfoMap`.
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
		int m_TicksPerSecond;
		std::vector<Bone> m_Bones;
		AssimpNodeData m_RootNode;
		std::map<std::string, BoneInfo> m_BoneInfoMap;
	};
}




