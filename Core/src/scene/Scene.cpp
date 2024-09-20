#include "Scene.h"

#include "../managers/EntityManager.h"
#include "../managers/LuaManager.h"
#include "../core/model/Transform.h"
#include "../core/textures/Material.h"
#include "../managers/SerializatorManager.h"

namespace libCore
{
    // Constructor que recibe el nombre de la escena
    Scene::Scene(const std::string& sceneName)
        : name(sceneName)
    {
    }

    //--PLAY LIFE CYCLE
    void Scene::Init()
    {
        EntityManager::GetInstance().InitScripts();
    }

    void Scene::Update(Timestep deltaTime)
    {
        EntityManager::GetInstance().UpdateScripts(deltaTime);
    }

    void Scene::Render(Timestep m_deltaTime)
    {
        // Lógica de renderización de la escena
    }
    //---------------------------------------------------------------------------------------------

}
