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


        void ReloadLuaScript(const std::string& scriptName);  // Nueva función para recargar el script y actualizar exposedVars


        // Declaraciones de las nuevas funciones para encapsular las exposedVars
        std::unordered_map<std::string, ExposedVar> GetExposedVars(const std::string& scriptName) const;

        void SetExposedVars(const std::string& scriptName, const std::unordered_map<std::string, ExposedVar>& vars);

        void LoadExposedVars(const std::string& scriptName);  // Cargar exposedVars desde Lua
        void SaveExposedVars(const std::string& scriptName);  // Guardar exposedVars a Lua

        //std::unordered_map<std::string, sol::object> GetExposedVars(const std::string& scriptName) const;
        //void SetExposedVars(const std::string& scriptName, const std::unordered_map<std::string, sol::object>& exposedVars);

        //--LIFE CYCLE
        void Init();
        void Update(float deltaTime);

        bool isReloading = false; // Indica si el script está en proceso de recarga

    private:
        

        entt::entity entity;  // Mantener esto si es necesario internamente
        uint32_t entityUUID;  // Exponer el UUID como uint32_t
        std::vector<ImportLUA_ScriptData> luaScriptsData;  // Cambiado a lista para múltiples scripts
        std::unordered_map<std::string, std::unordered_map<std::string, ExposedVar>> scriptExposedVars; // Almacena exposedVars por script
    };
}
