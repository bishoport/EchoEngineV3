#include "ScriptComponent.h"
#include "../../managers/EntityManager.h"
#include "../../managers/LuaManager.h"
#include "../../managers/AssetsManager.h"

namespace libCore
{
    //--LUA FILE MANAGMENT
    void ScriptComponent::AddLuaScript(const ImportLUA_ScriptData& scriptData) {
        luaScriptsData.push_back(scriptData);  // Agregar un nuevo script
    }

    void ScriptComponent::RemoveLuaScript(const std::string& scriptName) {
        luaScriptsData.erase(std::remove_if(luaScriptsData.begin(), luaScriptsData.end(),
            [&](const ImportLUA_ScriptData& data) { return data.name == scriptName; }), luaScriptsData.end());
    }

    //-------------------------------------------------------------------------------------------------------------------------

    //--LIFE CYCLE
    void ScriptComponent::Init() {
        for (const auto& scriptData : luaScriptsData) {
            sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptData.name);

            // Obtener la entidad desde el UUID
            entity = EntityManager::GetInstance().GetEntityByUUID(entityUUID);

            std::cout << "UUID PARENT -> " << entityUUID << std::endl;

            // Exponer el UUID de la entidad a Lua
            lua["entityUUID"] = static_cast<uint32_t>(entityUUID);  // Exponer el UUID como uint32_t
            lua["entity"] = entity;  // Exponer la entidad directamente

            sol::function initFunc = lua["Init"];
            if (initFunc.valid()) {
                initFunc();  // Ejecutar Init en cada script
            }
        }
    }


    void ScriptComponent::Update(float deltaTime) {
        for (const auto& scriptData : luaScriptsData) {
            sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptData.name);
            sol::function updateFunc = lua["Update"];
            if (updateFunc.valid()) {
                updateFunc(deltaTime);  // Ejecutar Update en cada script
            }
            else {
                std::cerr << "Lua Update function not found in script: " << scriptData.name << std::endl;
            }
        }
    }

    //-------------------------------------------------------------------------------------------------------------------------


    //--CUSTOM VARIABLES
    std::unordered_map<std::string, sol::object> ScriptComponent::GetExposedVars(const std::string& scriptName) const {
        sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
        sol::table exposedVars = lua["exposedVars"];

        std::unordered_map<std::string, sol::object> vars;

        if (exposedVars.valid()) {
            for (auto& pair : exposedVars) {
                std::string varName = pair.first.as<std::string>();
                sol::object varValue = pair.second;
                vars[varName] = varValue;  // Guardar los valores actuales
            }
        }
        return vars;
    }

    void ScriptComponent::SetExposedVars(const std::string& scriptName, const std::unordered_map<std::string, sol::object>& vars) {
        sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
        sol::table exposedVars = lua["exposedVars"];

        if (exposedVars.valid()) {
            for (const auto& [varName, varValue] : vars) {
                // Asegurarse de que el valor correcto se asigna a la tabla
                if (varValue.is<float>()) {
                    exposedVars[varName] = varValue.as<float>();
                }
                else if (varValue.is<int>()) {
                    exposedVars[varName] = varValue.as<int>();
                }
                else if (varValue.is<bool>()) {
                    exposedVars[varName] = varValue.as<bool>();
                }
                else if (varValue.is<std::string>()) {
                    exposedVars[varName] = varValue.as<std::string>();
                }
            }
        }
    }
    //-------------------------------------------------------------------------------------------------------------------------
}
