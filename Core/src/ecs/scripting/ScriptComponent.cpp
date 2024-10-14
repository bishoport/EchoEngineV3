#include "ScriptComponent.h"
#include "../../managers/EntityManager.h"
#include "../../managers/LuaManager.h"
#include "../../managers/AssetsManager.h"

namespace libCore
{
    //--LUA FILE MANAGMENT
    void ScriptComponent::AddLuaScript(const ImportLUA_ScriptData& scriptData) {
        luaScriptsData.push_back(scriptData);  // Agregar un nuevo script
        LoadExposedVars(scriptData.name);
    }

    void ScriptComponent::RemoveLuaScript(const std::string& scriptName) {
        luaScriptsData.erase(std::remove_if(luaScriptsData.begin(), luaScriptsData.end(),
            [&](const ImportLUA_ScriptData& data) { return data.name == scriptName; }), luaScriptsData.end());
    }

    void ScriptComponent::ReloadLuaScript(const std::string& scriptName) {
        // Inicia el proceso de recarga
        isReloading = true;

        // Recargar el archivo Lua desde el LuaManager
        LuaManager::GetInstance().ReloadLuaFile(scriptName);

        // Volver a cargar las exposedVars del script recargado
        LoadExposedVars(scriptName);

        // Finaliza el proceso de recarga
        isReloading = false;
    }

    //-------------------------------------------------------------------------------------------------------------------------

    //--LIFE CYCLE
    void ScriptComponent::Init() {
        for (const auto& scriptData : luaScriptsData) {
            sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptData.name);

            // Obtener la entidad desde el UUID
            entity = EntityManager::GetInstance().GetEntityByUUID(entityUUID);

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
            if (updateFunc.valid())
            {
                updateFunc(deltaTime);  // Ejecutar Update en cada script
            }
            else {
                std::cerr << "Lua Update function not found in script: " << scriptData.name << std::endl;
            }
        }
    }

    //-------------------------------------------------------------------------------------------------------------------------




    // Obtener las exposedVars del ScriptComponent (desde el almacenamiento)
    std::unordered_map<std::string, ExposedVar> ScriptComponent::GetExposedVars(const std::string& scriptName) const {
        auto it = scriptExposedVars.find(scriptName);
        if (it != scriptExposedVars.end()) {
            return it->second; // Retornar las exposedVars almacenadas para este script
        }
        return {}; // Retornar vacío si no se encontraron
    }

    // Establecer las exposedVars en el ScriptComponent
    void ScriptComponent::SetExposedVars(const std::string& scriptName, const std::unordered_map<std::string, ExposedVar>& vars) {
        scriptExposedVars[scriptName] = vars; // Almacenar las nuevas exposedVars para el script dado
    }

    // Cargar las exposedVars desde el script Lua al ScriptComponent
    void ScriptComponent::LoadExposedVars(const std::string& scriptName)
    {
        sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
        sol::table exposedVars = lua["exposedVars"];

        std::unordered_map<std::string, ExposedVar> vars;
        if (exposedVars.valid()) {
            for (auto& pair : exposedVars) {
                std::string varName = pair.first.as<std::string>();
                sol::object varValue = pair.second;

                // Almacenar las variables convertidas
                if (varValue.is<int>()) {
                    vars[varName] = ExposedVar{ varValue.as<int>() };
                }
                else if (varValue.is<float>()) {
                    vars[varName] = ExposedVar{ varValue.as<float>() };
                }
                else if (varValue.is<bool>()) {
                    vars[varName] = ExposedVar{ varValue.as<bool>() };
                }
                else if (varValue.is<std::string>()) {
                    vars[varName] = ExposedVar{ varValue.as<std::string>() };
                }
                else {
                    std::cerr << "Warning: Unrecognized type for variable " << varName << " in script " << scriptName << std::endl;
                }
            }
        }
        scriptExposedVars[scriptName] = vars; // Guardar en el ScriptComponent
    }

    // Guardar las exposedVars desde el ScriptComponent al estado Lua
    void ScriptComponent::SaveExposedVars(const std::string& scriptName) {
        sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
        sol::table exposedVars = lua["exposedVars"];

        if (exposedVars.valid()) {
            auto& vars = scriptExposedVars[scriptName]; // Obtener las vars almacenadas
            for (const auto& [varName, varValue] : vars) {
                std::visit([&](auto&& value) {
                    exposedVars[varName] = value;
                    }, varValue.value);
            }
        }
        else {
            std::cerr << "Error: No valid exposedVars table found for script: " << scriptName << std::endl;
        }
    }
}



////--CUSTOM VARIABLES
//std::unordered_map<std::string, sol::object> ScriptComponent::GetExposedVars(const std::string& scriptName) const {
//    sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
//    sol::table exposedVars = lua["exposedVars"];

//    std::unordered_map<std::string, sol::object> vars;

//    if (exposedVars.valid()) {
//        for (auto& pair : exposedVars) {
//            std::string varName = pair.first.as<std::string>();
//            sol::object varValue = pair.second;

//            // Detectar si es un UUID o int32
//            if (varName.rfind("uuid_", 0) == 0) {
//                vars[varName] = sol::make_object(lua, static_cast<int32_t>(varValue.as<uint32_t>()));
//            }
//            else if (varName.rfind("int_", 0) == 0) {
//                vars[varName] = sol::make_object(lua, varValue.as<int>());
//            }
//            else {
//                vars[varName] = varValue;  // Guardar los valores normales (float, bool, string)
//            }
//        }
//    }
//    return vars;
//}
//void ScriptComponent::SetExposedVars(const std::string& scriptName, const std::unordered_map<std::string, sol::object>& vars) 
//{
//    sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
//    sol::table exposedVars = lua["exposedVars"];

//    if (exposedVars.valid()) {
//        for (const auto& [varName, varValue] : vars) {
//            // Asegurarse de que el valor correcto se asigna a la tabla
//            if (varName.rfind("uuid_", 0) == 0 && varValue.is<int32_t>()) {
//                exposedVars[varName] = varValue.as<int32_t>();
//            }
//            else if (varName.rfind("int_", 0) == 0 && varValue.is<int>()) {
//                exposedVars[varName] = varValue.as<int>();
//            }
//            else if (varValue.is<float>()) {
//                exposedVars[varName] = varValue.as<float>();
//            }
//            else if (varValue.is<bool>()) {
//                exposedVars[varName] = varValue.as<bool>();
//            }
//            else if (varValue.is<std::string>()) {
//                exposedVars[varName] = varValue.as<std::string>();
//            }
//        }
//    }
//}
////-------------------------------------------------------------------------------------------------------------------------
