#include "EntityManagerBridge.h"
#include "../../managers/AssetsManager.h"
#include "../Components.h"  // Para los componentes como TagComponent

namespace libCore
{
    //--ENTITY MANAGER
    entt::entity EntityManagerBridge::CreateEntity(const std::string& name) {
        return EntityManager::GetInstance().CreateEmptyGameObject(name);
    }
    entt::entity EntityManagerBridge::CreateEntityFromModel(const std::string& modelName) {
        auto model = AssetsManager::GetInstance().GetModel(modelName);
        if (!model) {
            std::cerr << "Modelo no encontrado: " << modelName << std::endl;
            return entt::null;
        }
        return EntityManager::GetInstance().CreateGameObjectFromModel(model, entt::null);
    }

    entt::entity EntityManagerBridge::GetEntityByUUID(uint32_t uuid) 
    {
        return EntityManager::GetInstance().GetEntityByUUID(uuid);
    }



    void EntityManagerBridge::DestroyEntity(entt::entity entity) {
        EntityManager::GetInstance().DestroyEntity(entity);
    }
    std::string EntityManagerBridge::GetEntityName(entt::entity entity) {
        if (EntityManager::GetInstance().HasComponent<TagComponent>(entity)) {
            return EntityManager::GetInstance().GetComponent<TagComponent>(entity).Tag;
        }
        return "Unnamed";
    }
    //----------------------------------------------------------------------------------------------


    //--CHECK COMPONENTS
    template<typename T>
    bool EntityManagerBridge::HasComponent(entt::entity entity) {
        return EntityManager::GetInstance().HasComponent<T>(entity);
    }
    //----------------------------------------------------------------------------------------------


    //--TRANSFORM COMPONENT
    std::tuple<float, float, float> EntityManagerBridge::GetPosition(entt::entity entity) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            auto& transform = EntityManager::GetInstance().GetComponent<TransformComponent>(entity);
            return { transform.transform->position.x, transform.transform->position.y, transform.transform->position.z };
        }
        return { 0.0f, 0.0f, 0.0f };  // Valor por defecto si no tiene el componente
    }
    void EntityManagerBridge::SetPosition(entt::entity entity, float x, float y, float z) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            auto& transform = EntityManager::GetInstance().GetComponent<TransformComponent>(entity);
            transform.transform->position = { x, y, z };
        }
    }
    

    // Conversiones en EntityManagerBridge
    std::tuple<float, float, float> EntityManagerBridge::GetRotation(entt::entity entity) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            glm::vec3 eulerAngles = glm::degrees(EntityManager::GetInstance().GetComponent<TransformComponent>(entity).GetEulerAngles());
            return std::make_tuple(eulerAngles.x, eulerAngles.y, eulerAngles.z);
        }
        return std::make_tuple(0.0f, 0.0f, 0.0f);
    }

    void EntityManagerBridge::SetRotation(entt::entity entity, float x, float y, float z) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            EntityManager::GetInstance().GetComponent<TransformComponent>(entity).SetRotationEuler(glm::vec3(glm::radians(x), glm::radians(y), glm::radians(z)));
        }
    }

    /*std::tuple<float, float, float> EntityManagerBridge::GetRotation(entt::entity entity) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            glm::vec3 eulerAngles = EntityManager::GetInstance().GetComponent<TransformComponent>(entity).GetEulerAngles();
            return std::make_tuple(eulerAngles.x, eulerAngles.y, eulerAngles.z);
        }
        return std::make_tuple(0.0f, 0.0f, 0.0f);
    }
    void EntityManagerBridge::SetRotation(entt::entity entity, float x, float y, float z) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            EntityManager::GetInstance().GetComponent<TransformComponent>(entity).SetRotationEuler(glm::vec3(x, y, z));
        }
    }*/


    std::tuple<float, float, float> EntityManagerBridge::GetScale(entt::entity entity) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            auto& transform = EntityManager::GetInstance().GetComponent<TransformComponent>(entity);
            return { transform.transform->scale.x, transform.transform->scale.y, transform.transform->scale.z };
        }
        return { 1.0f, 1.0f, 1.0f };  // Escala por defecto si no tiene el componente
    }
    void EntityManagerBridge::SetScale(entt::entity entity, float x, float y, float z) {
        if (EntityManager::GetInstance().HasComponent<TransformComponent>(entity)) {
            auto& transform = EntityManager::GetInstance().GetComponent<TransformComponent>(entity);
            transform.transform->scale = { x, y, z };
        }
    }
    //----------------------------------------------------------------------------------------------

    //--
    void libCore::EntityManagerBridge::AddChild(entt::entity parent, entt::entity child)
    {
        EntityManager::GetInstance().AddChild(parent, child);
    }

}
