#pragma once

namespace libCore
{
    template<typename T>
    bool EntityManager::ValidateEntityComponent(entt::entity entity, Ref<entt::registry> registry) {
        if (!registry->valid(entity)) {
            std::cerr << "La entidad no es válida." << std::endl;
            return false;
        }

        if (!registry->has<T>(entity)) {
            std::cerr << "La entidad no tiene el componente requerido." << std::endl;
            return false;
        }

        auto& component = registry->get<T>(entity);
        if (!component) {
            std::cerr << "El componente está en un estado inválido (nullptr o no inicializado)." << std::endl;
            return false;
        }

        std::cout << "La entidad y el componente son válidos." << std::endl;
        return true;
    }

    template<typename T>
    T& EntityManager::GetComponent(entt::entity entity) 
    {
        return  EntityManager::GetInstance().GetRegistry()->get<T>(entity);
    }

    template<typename T>
    bool EntityManager::HasComponent(entt::entity entity) {
        return EntityManager::GetInstance().GetRegistry()->has<T>(entity);
    }

    template<typename T, typename... Args>
    T& EntityManager::AddComponent(entt::entity entity, Args&&... args) {
        return EntityManager::GetInstance().GetRegistry()->emplace<T>(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    void EntityManager::RemoveComponent(entt::entity entity) {
        EntityManager::GetInstance().GetRegistry()->remove<T>(entity);
    }
}

