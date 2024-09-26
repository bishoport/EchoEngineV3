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


		// Función para obtener la matriz de transformación de un hueso
		glm::mat4 GetBoneTransform(int boneID) const {
			for (const auto& [boneName, boneInfo] : m_BoneInfoMap) {
				if (boneInfo.id == boneID) 
				{
					return boneInfo.offset; // Devuelve la matriz de transformación (offset) del hueso
				}
			}
			return glm::mat4(1.0f);  // Devuelve la matriz identidad si no se encuentra el hueso
		}

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


		void DrawBones(const std::string& shaderName)
		{
			glLineWidth(2.0f); // Configuramos el grosor de las líneas

			// Dibujar líneas entre los huesos y sus padres
			for (const auto& [boneName, boneInfo] : m_BoneInfoMap) {
				if (boneInfo.parentId != -1) {
					// Obtener la posición del hueso actual y de su padre en el espacio mundial
					glm::vec3 currentBonePos = glm::vec3(GetBoneTransform(boneInfo.id) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					glm::vec3 parentBonePos = glm::vec3(GetBoneTransform(boneInfo.parentId) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

					//// Trazas para verificar las posiciones y huesos
					//std::cout << "Dibujando línea entre hueso: " << boneName
					//	<< " (ID: " << boneInfo.id << ") y su padre (ID: "
					//	<< boneInfo.parentId << ")." << std::endl;
					//std::cout << "Posición hueso: (" << currentBonePos.x << ", "
					//	<< currentBonePos.y << ", " << currentBonePos.z << ")" << std::endl;
					//std::cout << "Posición hueso padre: (" << parentBonePos.x << ", "
					//	<< parentBonePos.y << ", " << parentBonePos.z << ")" << std::endl;

					// Dibujar una línea entre el hueso y su padre
					glBegin(GL_LINES);
					glVertex3fv(glm::value_ptr(parentBonePos));
					glVertex3fv(glm::value_ptr(currentBonePos));
					glEnd();
				}
				else {
					/*std::cout << "Hueso: " << boneName << " (ID: " << boneInfo.id
						<< ") no tiene padre asignado (parentId = -1)." << std::endl;*/
				}
			}
		}



    };
}