#pragma once

#include "../core/Core.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../core/model/Model.h"

namespace libCore
{
	class ModelLoader 
	{
	public:
		static Ref<Model> LoadModel(ImportModelData importOptions);

	private:

		//Standard
		static void processNode(aiNode* node, const aiScene* scene, Ref<Model> modelParent, aiMatrix4x4 _nodeTransform, ImportModelData importOptions);
		static void processMesh(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild, aiMatrix4x4 finalTransform, ImportModelData importOptions, int meshIndex);
		static void processMaterials(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild, ImportModelData importOptions);

		//Skeletal
		static void processSkeletalMesh(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild, aiMatrix4x4 finalTransform, ImportModelData importOptions, int meshIndex);
		static void SetVertexBoneDataToDefault(Vertex& vertex);
		static void ExtractBoneWeightForVertices(Ref<Model> modelBuild, aiMesh* mesh, const aiScene* scene, int meshIndex);
		static void SetVertexBoneData(Vertex& vertex, int boneID, float weight);

		//Process Features
		static void processLights(const aiScene* scene);

		//Tools
		static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from);
		static aiMatrix4x4 glmToAiMatrix4x4(const glm::mat4& from);
	};
}