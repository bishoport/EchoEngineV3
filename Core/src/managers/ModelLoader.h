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
		static ModelLoader& getInstance() {
			static ModelLoader instance;
			return instance;
		}

		Ref<Model> LoadModel(ImportModelData importOptions);

	private:
		ModelLoader() {}

		std::unordered_map<unsigned int, Ref<Mesh>> processedMeshes;

		ImportModelData current_importOptions;//Cambia en cada importacion

		//Standard
		void processNode(aiNode* node, const aiScene* scene, Ref<Model> modelParent, aiMatrix4x4 _nodeTransform);
		void processMesh(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild, aiMatrix4x4 finalTransform, int meshIndex);
		void processMaterials(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild);

		//Process Features
		void processLights(const aiScene* scene);

		//Tools
		glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from);
		aiMatrix4x4 glmToAiMatrix4x4(const glm::mat4& from);
	};
}