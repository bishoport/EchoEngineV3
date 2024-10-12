#pragma once

#include "../../core/Core.h"
#include "../Components.h"
#include <entt.hpp>
#include <sol/sol.hpp>

namespace libCore
{
    class ScriptComponent {
    public:
        ScriptComponent() = default;
        ScriptComponent(uint32_t entityUUID) : entityUUID(entityUUID) {}

        void AddLuaScript(const ImportLUA_ScriptData& scriptData);

        bool HasLuaScripts() const { return !luaScriptsData.empty(); }

        const std::vector<ImportLUA_ScriptData>& GetLuaScriptsData() const { return luaScriptsData; }

        void RemoveLuaScript(const std::string& scriptName);

        void SetEntityUUID(uint32_t uuid) { this->entityUUID = uuid; }
        uint32_t GetEntityUUID() const { return entityUUID; }

        std::unordered_map<std::string, sol::object> GetExposedVars(const std::string& scriptName) const;

        // Establecer los valores de exposedVars de un script específico
        void SetExposedVars(const std::string& scriptName, const std::unordered_map<std::string, sol::object>& exposedVars);

        //--LIFE CYCLE
        void Init();
        void Update(float deltaTime);

    private:
        entt::entity entity;  // Mantener esto si es necesario internamente
        uint32_t entityUUID;  // Exponer el UUID como uint32_t
        std::vector<ImportLUA_ScriptData> luaScriptsData;  // Cambiado a lista para múltiples scripts
    };
}
