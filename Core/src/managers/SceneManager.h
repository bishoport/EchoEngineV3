#pragma once

#include "../core/Core.h"
#include "../scene/Scene.h"

namespace libCore
{
	class SceneManager
	{
	public:

		//--INSTANCE ACCESS
		static SceneManager& GetInstance()
		{
			static SceneManager instance;
			return instance;
		}

		//--CURRENT SCENE
		void CreateNewScene(const std::string& sceneName);
		Ref<Scene> GetCurrentScene() const{return currentScene;}
		void ClearCurrentScene();
		//---------------------------------------------------------

		//--SERIALIZACION
		void SaveScene();
		void LoadScene(std::string _sceneName);
		void SaveComponentsScene();
		void LoadComponentsScene();
		//---------------------------------------------------------


	private:
		// Constructor privado
		SceneManager() {}
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		~SceneManager() {}

		// La escena actualmente cargada
		Ref<Scene> currentScene;

		//--SERIALIZATION SCENE
		void SerializeScene();
		void DeserializeScene(std::string _sceneName);
		void SerializeComponents();
		void DeserializeComponents();
	};
}
