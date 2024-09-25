#pragma once

#include "../Core.h"

#include "Mesh.h"
#include "Transform.h"

#include "../textures/Material.h"
#include "../../managers/ShaderManager.h"
#include "skeletal/AnimData.h"

//#include "skeletal/AnimData.h"


namespace libCore
{
    class Model
    {
    public:

        // Constructor por defecto
        Model() = default;

        Model(ImportModelData _importModelData) : importModelData(_importModelData){}

        ImportModelData importModelData;
        Ref<Transform> transform = CreateRef<Transform>();
        std::string name = "no_name_model";


        //--MODEL DATA
        Ref<Model> modelParent = nullptr;
        std::vector<Ref<Model>> children;
        std::vector<Ref<Mesh>> meshes;
        std::vector<Ref<Material>> materials;




        //--SKELETAL DATA
		std::map<std::string, BoneInfo> m_BoneInfoMap;
		int m_BoneCounter = 0;

		auto& GetBoneInfoMap() { return m_BoneInfoMap; }
		int& GetBoneCount() { return m_BoneCounter; }


		void Draw(const std::string& shader)
		{
			//Bind Textures & Values
			for (unsigned int i = 0; i < materials.size(); i++)
			{

				//VALUES
				libCore::ShaderManager::Get(shader)->setVec3("albedoColor", materials[i]->albedoColor);
				libCore::ShaderManager::Get(shader)->setFloat("normalStrength", materials[i]->normalStrength);
				libCore::ShaderManager::Get(shader)->setFloat("metallicValue", materials[i]->metallicValue);
				libCore::ShaderManager::Get(shader)->setFloat("roughnessValue", materials[i]->roughnessValue);
				////libCore::ShaderManager::Get(shader)->setFloat("aoValue",        materials[i]->aoValue);

				//TEXTURES
				materials[i]->albedoMap->Bind(shader);
				materials[i]->normalMap->Bind(shader);
				materials[i]->metallicMap->Bind(shader);
				materials[i]->roughnessMap->Bind(shader);
				////materials[i]->aOMap->Bind(shader);
			}

			//Draw Meshes
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				libCore::ShaderManager::Get(shader)->setMat4("model", transform->getMatrix());
				meshes[i]->Draw();
				meshes[i]->DrawAABB();
			}
		}

    };
}