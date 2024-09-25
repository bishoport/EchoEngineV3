#pragma once
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../core/model/Model.h"
#include "../core/textures/Material.h"




namespace libCore
{
	class SkeletalModelLoader
	{
	public:

		static SkeletalModelLoader& getInstance() {
			static SkeletalModelLoader instance;
			return instance;
		}

		Ref<Model> LoadModel(ImportModelData importOptions);

	private:

		SkeletalModelLoader() {}

		ImportModelData current_importOptions;//Cambia en cada importacion

		void processNode(aiNode* node, const aiScene* scene, Ref<Model> modelParent, aiMatrix4x4 _nodeTransform);

		void processMaterials(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild);

		//Skeletal
		void processMesh(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild, aiMatrix4x4 finalTransform, int meshIndex);
		void SetVertexBoneDataToDefault(Vertex& vertex);
		void ExtractBoneWeightForVertices(Ref<Mesh> meshBuild, Ref<Model> modelBuild, aiMesh* mesh, const aiScene* scene);
		void SetVertexBoneData(Vertex& vertex, int boneID, float weight);


		//Tools
		glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from);
		aiMatrix4x4 glmToAiMatrix4x4(const glm::mat4& from);
		std::string getFileName(const std::string& path);
	};
}
