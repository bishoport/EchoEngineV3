#pragma once
#include "../core/Core.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/emitterstyle.h"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"

#include "../managers/AssetsManager.h"
#include "../core/model/Transform.h"
#include "../core/textures/Material.h"
#include "../core/textures/Texture.h"
#include "../core/model//Mesh.h"
#include "LuaManager.h"

#include "../ecs/Components.h"
#include "../ecs/scripting/ScriptComponent.h"

// YAML Conversion Specializations
namespace YAML 
{
    //--Vector3
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs) {
            if (!node.IsSequence() || node.size() != 3) {
                return false;
            }
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    //--Transform
    template<>
    struct convert<libCore::Transform> {
        static Node encode(const libCore::Transform& rhs) {
            Node node;
            node["Position"] = rhs.position;
            node["Rotation"] = rhs.eulerAngles;  // Se serializan los �ngulos de Euler
            node["Scale"] = rhs.scale;
            return node;
        }

        static bool decode(const Node& node, libCore::Transform& rhs) {
            if (!node["Position"] || !node["Rotation"] || !node["Scale"]) {
                return false;
            }
            rhs.position = node["Position"].as<glm::vec3>();
            rhs.eulerAngles = node["Rotation"].as<glm::vec3>();
            rhs.scale = node["Scale"].as<glm::vec3>();

            // Asegurarse de que la rotaci�n en cuaterni�n se actualice correctamente
            rhs.updateRotationFromEulerAngles();

            return true;
        }
    };

    //--Material
    template<>
    struct convert<libCore::Material> {
        static Node encode(const libCore::Material& rhs) {
            Node node;
            node["MaterialName"] = rhs.materialName;
            node["ShaderName"] = rhs.shaderName;
            node["AlbedoColor"] = rhs.albedoColor;
            node["NormalStrength"] = rhs.normalStrength;
            node["MetallicValue"] = rhs.metallicValue;
            node["RoughnessValue"] = rhs.roughnessValue;
            node["AoValue"] = rhs.aoValue;
            return node;
        }

        static bool decode(const Node& node, libCore::Material& rhs) {
            if (!node["MaterialName"] || !node["ShaderName"] || !node["AlbedoColor"] || !node["NormalStrength"] ||
                !node["MetallicValue"] || !node["RoughnessValue"] || !node["AoValue"]) {
                return false;
            }
            rhs.materialName = node["MaterialName"].as<std::string>();
            rhs.shaderName = node["ShaderName"].as<std::string>();
            rhs.albedoColor = node["AlbedoColor"].as<glm::vec3>();
            rhs.normalStrength = node["NormalStrength"].as<float>();
            rhs.metallicValue = node["MetallicValue"].as<float>();
            rhs.roughnessValue = node["RoughnessValue"].as<float>();
            rhs.aoValue = node["AoValue"].as<float>();
            return true;
        }
    };

    //--Texture
    template<>
    struct convert<libCore::Texture> {
        static Node encode(const libCore::Texture& rhs) {
            Node node;
            node["TexturePath"] = rhs.texturePath;
            node["TextureType"] = static_cast<int>(rhs.m_type);
            node["TextureUnit"] = rhs.m_unit;
            return node;
        }

        static bool decode(const Node& node, libCore::Texture& rhs) {
            if (!node["TexturePath"] || !node["TextureType"] || !node["TextureUnit"]) {
                return false;
            }
            rhs.texturePath = node["TexturePath"].as<std::string>();
            rhs.m_type = static_cast<TEXTURE_TYPES>(node["TextureType"].as<int>());
            rhs.m_unit = node["TextureUnit"].as<GLuint>();

            auto existingTexture = libCore::AssetsManager::GetInstance().GetTexture(rhs.texturePath);
            if (existingTexture && existingTexture->IsValid()) {
                rhs = *existingTexture;
            }
            else {
                //rhs = libCore::Texture(rhs.texturePath.c_str(), rhs.m_type, rhs.m_unit);
            }

            return true;
        }
    };

    //--UUID
    template<>
    struct convert<libCore::UUID> {
        static Node encode(const libCore::UUID& rhs) {
            Node node;
            node = rhs.ToString(); // Usar ToString para convertir el UUID a cadena
            return node;
        }

        static bool decode(const Node& node, libCore::UUID& rhs) {
            if (!node.IsScalar()) {
                return false;
            }
            rhs = libCore::UUID(node.as<std::string>()); // Reconstruir UUID desde la cadena
            return true;
        }
    };

    //--ImportModelData
    template<>
    struct convert<ImportModelData> {
        static Node encode(const ImportModelData& rhs) {
            Node node;
            node["filePath"] = rhs.filePath;
            node["fileName"] = rhs.fileName;
            node["modelID"] = rhs.modelID;
            node["invertUV"] = rhs.invertUV;
            node["rotate90"] = rhs.rotate90;
            node["skeletal"] = rhs.skeletal;
            node["useCustomTransform"] = rhs.useCustomTransform;
            node["processLights"] = rhs.processLights;
            node["globalScaleFactor"] = rhs.globalScaleFactor;
            return node;
        }

        static bool decode(const Node& node, ImportModelData& rhs) {
            if (!node["filePath"] || !node["fileName"]) {
                return false;
            }
            rhs.filePath = node["filePath"].as<std::string>();
            rhs.fileName = node["fileName"].as<std::string>();
            rhs.modelID  = node["modelID"].as<int>();
            rhs.invertUV = node["invertUV"].as<bool>();
            rhs.rotate90 = node["rotate90"].as<bool>();
            rhs.skeletal = node["skeletal"].as<bool>();
            rhs.useCustomTransform = node["useCustomTransform"].as<bool>();
            rhs.processLights = node["processLights"].as<bool>();
            rhs.globalScaleFactor = node["globalScaleFactor"].as<float>();
            return true;
        }
    };

    //--ImportLUA_ScriptData YAML specialization
    template<>
    struct convert<ImportLUA_ScriptData> {
        static Node encode(const ImportLUA_ScriptData& rhs) {
            Node node;
            node["filePath"] = rhs.filePath;
            node["name"] = rhs.name;
            return node;
        }

        static bool decode(const Node& node, ImportLUA_ScriptData& rhs) {
            if (!node["filePath"] || !node["name"]) {
                return false;
            }
            rhs.filePath = node["filePath"].as<std::string>();
            rhs.name = node["name"].as<std::string>();
            return true;
        }
    };
}



// Serialization and Deserialization Functions
namespace libCore {

    //--TransformComponent
    YAML::Node SerializeTransformComponent(const TransformComponent& transformComponent) {
        YAML::Node node;
        node["Transform"] = *transformComponent.transform;
        return node;
    }
    TransformComponent DeserializeTransformComponent(const YAML::Node& node) {
        TransformComponent transformComponent;
        transformComponent.transform = CreateRef<Transform>(node["Transform"].as<Transform>());

        // La funci�n 'as<Transform>()' ya maneja la actualizaci�n del cuaterni�n desde los �ngulos de Euler.
        return transformComponent;
    }
    //--------------------

    //--MaterialComponent
    YAML::Node SerializeMaterialComponent(const MaterialComponent& materialComponent) {
        YAML::Node node;
        node["Material"] = *materialComponent.material;

        if (materialComponent.material->albedoMap) {
            node["AlbedoMap"] = *materialComponent.material->albedoMap;
        }
        if (materialComponent.material->normalMap) {
            node["NormalMap"] = *materialComponent.material->normalMap;
        }
        if (materialComponent.material->metallicMap) {
            node["MetallicMap"] = *materialComponent.material->metallicMap;
        }
        if (materialComponent.material->roughnessMap) {
            node["RoughnessMap"] = *materialComponent.material->roughnessMap;
        }
        if (materialComponent.material->aOMap) {
            node["AOMap"] = *materialComponent.material->aOMap;
        }

        return node;
    }
    MaterialComponent DeserializeMaterialComponent(const YAML::Node& node) {
        MaterialComponent materialComponent;
        materialComponent.material = CreateRef<Material>();
        *materialComponent.material = node["Material"].as<Material>();

        if (node["AlbedoMap"]) {
            materialComponent.material->albedoMap = CreateRef<Texture>();
            *materialComponent.material->albedoMap = node["AlbedoMap"].as<Texture>();
        }
        if (node["NormalMap"]) {
            materialComponent.material->normalMap = CreateRef<Texture>();
            *materialComponent.material->normalMap = node["NormalMap"].as<Texture>();
        }
        if (node["MetallicMap"]) {
            materialComponent.material->metallicMap = CreateRef<Texture>();
            *materialComponent.material->metallicMap = node["MetallicMap"].as<Texture>();
        }
        if (node["RoughnessMap"]) {
            materialComponent.material->roughnessMap = CreateRef<Texture>();
            *materialComponent.material->roughnessMap = node["RoughnessMap"].as<Texture>();
        }
        if (node["AOMap"]) {
            materialComponent.material->aOMap = CreateRef<Texture>();
            *materialComponent.material->aOMap = node["AOMap"].as<Texture>();
        }

        return materialComponent;
    }
    //--------------------


    //--MeshComponent
    YAML::Node SerializeMeshComponent(const MeshComponent& meshComponent) {
        YAML::Node node;
        node["MeshName"] = meshComponent.mesh->meshName;
        return node;
    }
    MeshComponent DeserializeMeshComponent(const YAML::Node& node) {
        MeshComponent meshComponent;
        meshComponent.mesh = AssetsManager::GetInstance().GetMesh(node["MeshName"].as<std::string>());
        return meshComponent;
    }
    //--------------------

    //--AABBComponent
    YAML::Node SerializeAABBComponent(const AABBComponent& aabbComponent) {
        YAML::Node node;
        node["AABBMin"] = aabbComponent.aabb->minBounds;
        node["AABBMax"] = aabbComponent.aabb->maxBounds;
        return node;
    }
    AABBComponent DeserializeAABBComponent(const YAML::Node& node) {
        AABBComponent aabbComponent;
        aabbComponent.aabb->minBounds = node["AABBMin"].as<glm::vec3>();
        aabbComponent.aabb->maxBounds = node["AABBMax"].as<glm::vec3>();
        return aabbComponent;
    }
    //--------------------

    //--MODEL Component
    YAML::Node SerializeModel(const Ref<Model>& model) {
        YAML::Node node;
        node["ModelName"] = model->name;
        node["ImportModelData"] = model->importModelData;
        return node;
    }
    YAML::Node SerializeAllModels() {
        YAML::Node node;
        for (const auto& pair : AssetsManager::GetInstance().GetAllModels()) {
            node.push_back(SerializeModel(pair.second));
        }
        return node;
    }
    Ref<Model> DeserializeModel(const YAML::Node& node) {
        std::string modelName = node["ModelName"].as<std::string>();
        ImportModelData importData = node["ImportModelData"].as<ImportModelData>();

        AssetsManager::GetInstance().LoadModelAsset(importData);
        // Cargar el modelo desde el archivo usando ImportModelData
        // Suponiendo que tengas una funci�n que carga un modelo dado `ImportModelData`


        return AssetsManager::GetInstance().GetModel(modelName);
    }
    void DeserializeAllModels(const YAML::Node& node) {
        for (const auto& modelNode : node) {
            DeserializeModel(modelNode);
        }
    }
    //--------------------


    //--IDComponent
    YAML::Node SerializeIDComponent(const IDComponent& idComponent) {
        YAML::Node node;
        node["ID"] = idComponent.ID;
        node["MarkToDelete"] = idComponent.markToDelete;
        return node;
    }
    IDComponent DeserializeIDComponent(const YAML::Node& node) {
        IDComponent idComponent;
        idComponent.ID = node["ID"].as<UUID>();
        idComponent.markToDelete = node["MarkToDelete"].as<bool>();
        return idComponent;
    }
    //--------------------

    //--TagComponent
    YAML::Node SerializeTagComponent(const TagComponent& tagComponent) {
        YAML::Node node;
        node["Tag"] = tagComponent.Tag;
        return node;
    }
    TagComponent DeserializeTagComponent(const YAML::Node& node) {
        TagComponent tagComponent;
        tagComponent.Tag = node["Tag"].as<std::string>();
        return tagComponent;
    }
    //--------------------


    //--ScriptComponent
    YAML::Node SerializeLUA_Script(const ImportLUA_ScriptData& scriptData) {
        YAML::Node node;
        node["filePath"] = scriptData.filePath;
        node["name"] = scriptData.name;
        return node;
    }
    YAML::Node SerializeAllLUAScripts(const std::vector<ImportLUA_ScriptData>& scripts) {
        YAML::Node node;
        for (const auto& scriptData : scripts) {
            node.push_back(SerializeLUA_Script(scriptData));
        }
        return node;
    }
    void DeserializeAllLUAScripts(const YAML::Node& node, ScriptComponent& scriptComponent) {
        for (const auto& scriptNode : node) {
            ImportLUA_ScriptData scriptData;
            scriptData.filePath = scriptNode["filePath"].as<std::string>();
            scriptData.name = scriptNode["name"].as<std::string>();

            // Cargar el script en LuaManager y agregarlo al componente de script
            LuaManager::GetInstance().LoadLuaFile(scriptData.name, scriptData.filePath);
            scriptComponent.AddLuaScript(scriptData);
        }
    }
    YAML::Node SerializeScriptComponent(const ScriptComponent& scriptComponent) {
        YAML::Node node;

        // Serializar los datos de todos los scripts
        auto luaScripts = scriptComponent.GetLuaScriptsData();
        node["Scripts"] = SerializeAllLUAScripts(luaScripts);

        // Serializar los valores de exposedVars para cada script
        YAML::Node scriptsVarsNode;
        for (const auto& scriptData : luaScripts) {
            YAML::Node exposedVarsNode;
            auto exposedVars = scriptComponent.GetExposedVars(scriptData.name);
            for (const auto& [varName, varValue] : exposedVars) {
                if (varValue.is<float>()) {
                    exposedVarsNode[varName] = varValue.as<float>();
                }
                else if (varValue.is<int>()) {
                    exposedVarsNode[varName] = varValue.as<int>();
                }
                else if (varValue.is<bool>()) {
                    exposedVarsNode[varName] = varValue.as<bool>();
                }
                else if (varValue.is<std::string>()) {
                    exposedVarsNode[varName] = varValue.as<std::string>();
                }
            }
            scriptsVarsNode[scriptData.name] = exposedVarsNode;
        }
        node["ExposedVars"] = scriptsVarsNode;

        return node;
    }
    ScriptComponent DeserializeScriptComponent(const YAML::Node& node) {
        ScriptComponent scriptComponent;

        // Deserializar todos los scripts
        if (node["Scripts"]) {
            DeserializeAllLUAScripts(node["Scripts"], scriptComponent);
        }

        // Verificar si existen las variables exposedVars para cada script
        if (node["ExposedVars"]) {
            const auto& scriptsVarsNode = node["ExposedVars"];
            for (const auto& scriptNode : scriptsVarsNode) {
                std::string scriptName = scriptNode.first.as<std::string>();
                sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptName);
                sol::table exposedVars = lua["exposedVars"];

                std::unordered_map<std::string, sol::object> vars;

                const auto& exposedVarsNode = scriptNode.second;
                for (const auto& varNode : exposedVarsNode) {
                    std::string varName = varNode.first.as<std::string>();

                    // Detectar el tipo basado en el prefijo del nombre de la variable
                    if (varName.rfind("int_", 0) == 0) {  // Si el nombre tiene el prefijo "int_"
                        vars[varName] = sol::make_object(lua, varNode.second.as<int>());
                    }
                    else if (varNode.second.IsScalar() && varNode.second.Tag() == "!!float") {
                        vars[varName] = sol::make_object(lua, varNode.second.as<float>());
                    }
                    else if (varNode.second.IsScalar() && varNode.second.Tag() == "!!bool") {
                        vars[varName] = sol::make_object(lua, varNode.second.as<bool>());
                    }
                    else if (varNode.second.IsScalar()) {
                        vars[varName] = sol::make_object(lua, varNode.second.as<std::string>());
                    }
                }

                // Actualiza los valores de exposedVars en ScriptComponent con un unordered_map
                scriptComponent.SetExposedVars(scriptName, vars);
            }
        }

        return scriptComponent;
    }
    //--------------------
}
