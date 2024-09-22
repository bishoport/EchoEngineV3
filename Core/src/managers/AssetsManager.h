#pragma once

#include "../core/Core.h"
#include "../core/textures/Texture.h"
#include "../core/model/Model.h"

#include <future> // Para std::future y std::promise
#include <atomic> // Para manejar el progreso de forma segura entre hilos

namespace libCore
{
	class AssetsManager
	{
	public:

		// Método para obtener la instancia de la clase
		static AssetsManager& GetInstance() 
		{
			static AssetsManager instance;
			return instance;
		}

		//--TEXTURES

		std::string defaultAssetsPathTexture = "C:/Users/bisho/OneDrive/Escritorio/OPENGL_GAME/OpenGL_ShooterV2/Core/assets/textures/";
		void UnloadTexture(const std::string& name);
		void LoadDefaultAssets();
		Ref<Texture> LoadTextureAsset(const std::string& key, const char* directoryPath, const char* fileName, TEXTURE_TYPES type);
		Ref<Texture> GetTexture(const std::string& name);
		void SetTexture(const std::string& name, const Ref<Texture>& texture);
		const std::unordered_map<std::string, Ref<Texture>>& GetAllTextures() const;
		std::size_t GetNumberOfTextures() const;
		//---------------------------------------------------------------------------------------------------------------------------


		//--MODELS
		void LoadModelAsset(ImportModelData importModelData);
		Ref<Model> GetModel(const std::string& name);
		Ref<Model> FindModelRecursive(const Ref<Model>& model, const std::string& name);
		Ref<Model> GetModelByMeshName(const std::string& meshName);
		const std::unordered_map<std::string, Ref<Model>>& GetAllModels() const;
		Ref<Mesh> GetMesh(const std::string& name);
		//---------------------------------------------------------------------------------------------------------------------------

		

		//--MATERIALS
		void CreateDefaultMaterial();
		void CreateEmptyMaterial(const std::string& matName);
		Ref<Material> addMaterial(Ref<Material> materialData);
		Ref<Material> getMaterial(const std::string& key);
		bool removeMaterial(const std::string& key);
		const std::unordered_map<std::string, Ref<Material>>& GetAllMaterials() const;
		//---------------------------------------------------------------------------------------------------------------------------

	private:
		// Constructor privado
		AssetsManager() 
		{
			std::filesystem::path basePath = std::filesystem::path("../../");
			std::filesystem::path fullPath = basePath / "path/to/your/file.txt";
			std::string fullPathStr = fullPath.string();
		}

		//-CARGADOS EN MEMORIA
		std::unordered_map<std::string, Ref<Material>> loadedMaterials;
		std::unordered_map<std::string, Ref<Texture>>  loadedTextures;
		std::unordered_map<std::string, Ref<Model>>    loadedModels;


		// Eliminar la posibilidad de copiar o asignar instancias
		AssetsManager(const AssetsManager&) = delete;
		AssetsManager& operator=(const AssetsManager&) = delete;
	};
}
