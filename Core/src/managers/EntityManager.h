#pragma once

#include <entt.hpp>

#include "../core/Core.h"
#include "../core/Timestep.h"

#include "../ecs/Components.h"
#include "../ecs/scripting/ScriptComponent.h"




namespace libCore
{
    class EntityManager {
    public:

        static EntityManager& GetInstance();

        bool runScripting = false;

        std::unordered_set<uint32_t> renderableObjects;

        Scope<entt::registry> m_registry;

        std::vector<entt::entity> entitiesInRay;
        entt::entity currentSelectedEntityInScene = entt::null;

        entt::entity GetEntityByUUID(uint32_t uuid);
        entt::registry* GetRegistry();

        entt::entity CreateEmptyGameObject(const std::string& name = "");
        entt::entity CreateGameObjectFromModel(Ref<Model> model, entt::entity parent);
        void CreateCamera();
        void AddChild(entt::entity parent, entt::entity child);

        void DuplicateEntity();
        entt::entity DuplicateEntityRecursively(entt::entity entity, entt::entity parentEntity);
        bool IsTagUsed(const std::string& tag);
        void UpdateAccumulatedTransforms(entt::entity entity, const glm::mat4& parentTransform = glm::mat4(1.0f));

        void MarkToDeleteRecursively(entt::entity entity);
        void DestroyEntity(entt::entity entity);
        void DestroyEntityRecursively(entt::entity entity);
        void DestroyAllEntities();

        void InitScripts();
        void UpdateScripts(Timestep deltaTime);
        void UpdateGameObjects(Timestep deltaTime);
        void DrawGameObjects(const std::string& shader, const int viewportNumber);
        void DrawOneGameObject(entt::entity entity, const std::string& shader);
        void DrawABBGameObjectMeshComponent(const std::string& shader);
        void CheckRayModelIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
        void CheckInstancesInRunTime();
        bool rayIntersectsBoundingBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::vec3 boxMin, glm::vec3 boxMax);

        glm::vec3 UUIDToColor(const UUID& uuid);
        uint32_t ColorToUUID(unsigned char r, unsigned char g, unsigned char b);


        //--TWEEN SYSTEM
        void MoveEntityWithTween(entt::entity entity, glm::vec3 targetPosition, float duration);
        void ScaleEntityWithTween(entt::entity entity, glm::vec3 targetScale, float duration);
        void RotateEntityWithTween(entt::entity entity, glm::vec3 targetRotationEuler, float duration);


        //--TEMPLATES
        template<typename T>
        bool ValidateEntityComponent(entt::entity entity, Ref<entt::registry> registry);

        template<typename T>
        T& GetComponent(entt::entity entity);

        template<typename T>
        bool HasComponent(entt::entity entity);

        template<typename T, typename... Args>
        T& AddComponent(entt::entity entity, Args&&... args);

        template<typename T>
        void RemoveComponent(entt::entity entity);
        //-------------------------------------------------------------------------------------
        

    private:
        EntityManager();

    public:
        void PrintEntityInfo(entt::entity entity, const Ref<entt::registry>& registry);
        void DebugPrintAllEntities();
        void DebugPrintEntityHierarchy(entt::entity entity, const Ref<entt::registry>& registry, int level = 0);
        void DebugPrintAllEntityHierarchies();
        void DebugPrintAllEntitiesWithUUIDs();
        void DestroyDeleteMarked();
    };
}

#include "EntityManager.inl"
