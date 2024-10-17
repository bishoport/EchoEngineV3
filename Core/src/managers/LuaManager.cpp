// LuaManager.cpp
#include "LuaManager.h"
#include "EntityManager.h"
#include "../ecs/scripting/EntityManagerBridge.h"

namespace libCore {

    void LuaManager::LoadLuaFile(const std::string& scriptName, const std::string& path)
    {
        auto luaState = std::make_unique<sol::state>();
        luaState->open_libraries(sol::lib::base);

        // Registrar las funciones comunes
        RegisterCommonFunctions(*luaState);

        try {
            luaState->script_file(path);

            // Llenar la estructura con los datos del script
            ImportLUA_ScriptData scriptData;
            scriptData.name = scriptName;
            scriptData.filePath = path;

            // Guardar el script en el mapa
            scripts[scriptName] = std::make_pair(scriptData, std::move(luaState));

            ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Loaded Lua script: " + scriptName + " from path: " + path);
        }
        catch (const sol::error& e) {
            ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Error loading Lua script:" + std::string(e.what()));
        }
    }
    void LuaManager::ReloadLuaFile(const std::string& scriptName)
    {
        reloading = true;

        auto it = scripts.find(scriptName);
        if (it != scripts.end()) {
            const std::string& path = it->second.first.filePath;

            auto luaState = std::make_unique<sol::state>();
            luaState->open_libraries(sol::lib::base);
            RegisterCommonFunctions(*luaState); // Registrar funciones comunes nuevamente

            try {
                luaState->script_file(path);  // Vuelve a cargar el archivo Lua
                it->second.second = std::move(luaState);  // Reemplaza el estado Lua con el nuevo

                ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Reloaded Lua script:" + std::string(path));
            }
            catch (const sol::error& e) {
                ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Error reloading Lua script:" + std::string(e.what()));
            }
        }
        else 
        {
            ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Script not found to reload::" + std::string(scriptName));
        }

        reloading = false;
    }


    void LuaManager::RegisterCommonFunctions(sol::state& luaState) {

        luaState.new_usertype<entt::entity>("entity");

        luaState.new_usertype<EntityManagerBridge>("EntityManagerBridge",
            "CreateEntity",          &EntityManagerBridge::CreateEntity,
            "CreateEntityFromModel", &EntityManagerBridge::CreateEntityFromModel,
            "GetEntityName",         &EntityManagerBridge::GetEntityName,
            "DestroyEntity",         &EntityManagerBridge::DestroyEntity,
            "GetEntityByUUID",       &EntityManagerBridge::GetEntityByUUID, // Exponer GetEntityByUUID

            "GetPosition",   &EntityManagerBridge::GetPosition,
            "SetPosition",   &EntityManagerBridge::SetPosition,
            "GetRotation",   &EntityManagerBridge::GetRotation,
            "SetRotation",   &EntityManagerBridge::SetRotation,
            "GetScale",      &EntityManagerBridge::GetScale,
            "SetScale",      &EntityManagerBridge::SetScale,

            "AddChild", &EntityManagerBridge::AddChild,
            "MoveEntityWithTween", & EntityManagerBridge::MoveEntityWithTween
        );


        static EntityManagerBridge entityManagerBridge;
        luaState["EntityManager"] = &entityManagerBridge;
    }


    sol::state& LuaManager::GetLuaState(const std::string& scriptName) const {
        auto it = scripts.find(scriptName);
        if (it != scripts.end()) {
            return *(it->second.second);  // Devuelve la referencia del estado Lua
        }
        throw std::runtime_error("Lua state not found for script: " + scriptName);
    }
    bool LuaManager::IsScriptLoaded(const std::string& scriptName) const {
        return scripts.find(scriptName) != scripts.end();
    }
    std::vector<ImportLUA_ScriptData> LuaManager::GetLoadedScripts() const {
        std::vector<ImportLUA_ScriptData> scriptDataList;
        for (const auto& pair : scripts) {
            scriptDataList.push_back(pair.second.first);
        }
        return scriptDataList;
    }


    ImportLUA_ScriptData LuaManager::GetScriptData(const std::string& scriptName) const {
        auto it = scripts.find(scriptName);
        if (it != scripts.end()) {
            return it->second.first;
        }
        throw std::runtime_error("Script data not found for script: " + scriptName);
    }
}
