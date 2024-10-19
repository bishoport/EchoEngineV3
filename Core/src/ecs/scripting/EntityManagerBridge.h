#pragma once

#include "../../managers/EntityManager.h"  // Incluir la clase original de EntityManager
#include <tuple>

namespace libCore {

    class EntityManagerBridge {
    public:

        //--ENTITES
        entt::entity CreateEntity(const std::string& name);
        entt::entity CreateEntityFromModel(const std::string& modelName);
        void DestroyEntity(entt::entity entity);
        std::string GetEntityName(entt::entity entity);
        //----------------------------------------------------------------------------------------

        entt::entity GetEntityByUUID(uint32_t uuid);

        //--CHECK COMPONENTS
        template<typename T>
        bool HasComponent(entt::entity entity);
        //----------------------------------------------------------------------------------------

        //--TRANSFORM COMPONENT
        std::tuple<float, float, float> GetPosition(entt::entity entity);
        void SetPosition(entt::entity entity, float x, float y, float z);
        std::tuple<float, float, float> GetRotation(entt::entity entity);  // Rotación en Euler
        void SetRotation(entt::entity entity, float x, float y, float z);  // Establecer rotación en Euler
        std::tuple<float, float, float> GetScale(entt::entity entity);
        void SetScale(entt::entity entity, float x, float y, float z);
        //----------------------------------------------------------------------------------------

        //--Entity Managment
        void AddChild(entt::entity parent, entt::entity child);
        //----------------------------------------------------------------------------------------

        //--TWEEN
        void MoveEntityWithTween(entt::entity entity, float x, float y, float z, float duration);
        void ScaleEntityWithTween(entt::entity entity, float x, float y, float z, float duration);
        void RotateEntityWithTween(entt::entity entity, float x, float y, float z, float duration);
        //----------------------------------------------------------------------------------------


        //--GRID MANAGER
        std::vector<std::vector<int>> GetGridLayerAsMatrix(const std::string& gridName, const std::string& layerName);
        //----------------------------------------------------------------------------------------


    };
}
