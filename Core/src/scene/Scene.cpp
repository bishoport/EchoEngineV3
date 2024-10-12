#include "Scene.h"

#include "../managers/SerializatorManager.h"

#include "../managers/EntityManager.h"
#include "../managers/LuaManager.h"

namespace libCore
{
    // Constructor que recibe el nombre de la escena
    Scene::Scene(const std::string& name): sceneName(name){}
    //---------------------------------------------------------------------------------------------

    //--SAVE/LOAD PROJECT
    void Scene::SerializeScene() {

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";

        // Serializar los modelos cargados en memoria
        auto models = SerializeAllModels();
        if (models.size() > 0) {
            out << YAML::Key << "Models" << YAML::Value << models;
        }
        else {
            out << YAML::Key << "Models" << YAML::Value << YAML::BeginSeq << YAML::EndSeq; // Lista vacía
        }


        // Serializar las entidades
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        auto& entityManager = EntityManager::GetInstance();
        entityManager.m_registry->each([&](auto entity) {

            out << YAML::BeginMap; // Entity
            out << YAML::Key << "Entity" << YAML::Value << (uint32_t)entity; // Serialize entity ID

            if (entityManager.HasComponent<IDComponent>(entity)) {
                out << YAML::Key << "IDComponent";
                out << YAML::Value << SerializeIDComponent(entityManager.GetComponent<IDComponent>(entity));
            }

            if (entityManager.HasComponent<TagComponent>(entity)) {
                out << YAML::Key << "TagComponent";
                out << YAML::Value << SerializeTagComponent(entityManager.GetComponent<TagComponent>(entity));
            }

            // Serializar TransformComponent
            if (entityManager.HasComponent<TransformComponent>(entity)) {
                out << YAML::Key << "TransformComponent";
                out << YAML::Value << SerializeTransformComponent(entityManager.GetComponent<TransformComponent>(entity));
            }

            // Serializar el ParentComponent
            if (entityManager.HasComponent<ParentComponent>(entity)) {
                out << YAML::Key << "ParentComponent";
                auto& parentComponent = entityManager.GetComponent<ParentComponent>(entity);
                out << YAML::BeginMap;
                out << YAML::Key << "Parent" << YAML::Value << (uint32_t)parentComponent.parent;
                out << YAML::EndMap;
            }

            // Serializar el ChildComponent
            if (entityManager.HasComponent<ChildComponent>(entity)) {
                out << YAML::Key << "ChildComponent";
                auto& childComponent = entityManager.GetComponent<ChildComponent>(entity);
                out << YAML::BeginSeq;
                for (auto child : childComponent.children) {
                    out << (uint32_t)child;
                }
                out << YAML::EndSeq;
            }

            // Serializar MeshComponent
            if (entityManager.HasComponent<MeshComponent>(entity)) {
                out << YAML::Key << "MeshComponent";
                out << YAML::Value << SerializeMeshComponent(entityManager.GetComponent<MeshComponent>(entity));
            }

            // Serializar MaterialComponent
            if (entityManager.HasComponent<MaterialComponent>(entity)) {
                out << YAML::Key << "MaterialComponent";
                out << YAML::Value << SerializeMaterialComponent(entityManager.GetComponent<MaterialComponent>(entity));
            }

            // Serializar ScriptComponent con múltiples scripts
            if (entityManager.HasComponent<ScriptComponent>(entity)) {
                out << YAML::Key << "ScriptComponent";
                out << YAML::Value << SerializeScriptComponent(entityManager.GetComponent<ScriptComponent>(entity));
            }

            out << YAML::EndMap; // Entity
            });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout("assets/Scenes/" + sceneName + ".yaml");
        fout << out.c_str();
    }
    void Scene::DeserializeScene(const std::string& _sceneName) {
       
        sceneName = _sceneName;

        YAML::Node data = YAML::LoadFile("assets/Scenes/" + _sceneName + ".yaml");
        
        if (!data["Scene"]) {
            return;
        }

        // Deserializar los modelos cargados en memoria
        if (data["Models"]) {
            DeserializeAllModels(data["Models"]);
        }

        auto& entityManager = EntityManager::GetInstance();
        auto entities = data["Entities"];

        // Mapa para mantener la correspondencia entre los IDs de las entidades y las entidades deserializadas
        std::unordered_map<uint32_t, entt::entity> entityMap;

        if (entities) {
            // Primer pase: Crear todas las entidades y deserializar los componentes básicos
            for (auto entityNode : entities) {
                uint32_t entityID = entityNode["Entity"].as<uint32_t>();

                entt::entity entity = entityManager.CreateEmptyGameObject();

                if (entityNode["TagComponent"]) {
                    auto tag_component = DeserializeTagComponent(entityNode["TagComponent"]);
                    entityManager.m_registry->emplace_or_replace<TagComponent>(entity, tag_component);
                }

                if (entityNode["IDComponent"]) {
                    // Deserializar el componente IDComponent
                    auto id_component = DeserializeIDComponent(entityNode["IDComponent"]);

                    // Obtener el UUID desde el IDComponent deserializado
                    uint32_t entityUUID = static_cast<uint32_t>(id_component.ID);

                    // Asignar o reemplazar el IDComponent en la entidad con el UUID deserializado
                    entityManager.m_registry->emplace_or_replace<IDComponent>(entity, id_component);
                }


                if (entityNode["TransformComponent"]) {
                    auto transform_component = DeserializeTransformComponent(entityNode["TransformComponent"]);
                    entityManager.m_registry->emplace_or_replace<TransformComponent>(entity, transform_component);
                }

                if (entityNode["MeshComponent"]) {
                    std::string meshModelName = entityNode["MeshComponent"]["MeshName"].as<std::string>();
                    Ref<Model> model = AssetsManager::GetInstance().GetModelByMeshName(meshModelName);

                    if (model != nullptr) {
                        // Asignar los componentes de MeshComponent si el modelo tiene mallas
                        if (!model->meshes.empty()) {
                            for (auto& mesh : model->meshes) {
                                auto& meshComponent = entityManager.m_registry->emplace<MeshComponent>(entity, mesh, model, true);
                                glm::mat4 instanceMatrix = glm::translate(glm::mat4(1.0f), model->transform->position);
                                meshComponent.instanceMatrices.push_back(instanceMatrix);
                                auto& abbComponent = entityManager.m_registry->emplace<AABBComponent>(entity);
                                abbComponent.aabb->CalculateAABB(mesh->vertices);
                            }
                        }
                    }
                    else {
                        std::cout << "ERROR AL CARGAR EL MODELO con la mesh " << meshModelName << std::endl;
                        continue;
                    }
                }

                if (entityNode["MaterialComponent"]) {
                    auto component = DeserializeMaterialComponent(entityNode["MaterialComponent"]);
                    Ref<Material> mat = AssetsManager::GetInstance().getMaterial(component.material->materialName);
                    if (mat != nullptr) {
                        component.material = mat;
                    }
                    entityManager.m_registry->emplace_or_replace<MaterialComponent>(entity, component);
                }

                // Deserializar ScriptComponent con múltiples scripts
                if (entityNode["ScriptComponent"]) {
                    auto component = DeserializeScriptComponent(entityNode["ScriptComponent"]);

                    // Obtener el UUID desde el IDComponent deserializado (ya deserializado antes de este punto)
                    if (entityNode["IDComponent"]) {
                        auto id_component = DeserializeIDComponent(entityNode["IDComponent"]);

                        // Obtener el UUID desde el IDComponent deserializado
                        uint32_t entityUUID = static_cast<uint32_t>(id_component.ID);

                        // Ahora asignamos ese UUID al ScriptComponent
                        component.SetEntityUUID(entityUUID);
                    }

                    entityManager.m_registry->emplace_or_replace<ScriptComponent>(entity, component);
                }


                // Almacenar la correspondencia entre el ID de la entidad y la entidad creada
                entityMap[entityID] = entity;
            }

            // Segundo pase: Configurar las relaciones de jerarquía (padres e hijos)
            for (auto entityNode : entities) {
                uint32_t entityID = entityNode["Entity"].as<uint32_t>();
                entt::entity entity = entityMap[entityID];

                // Deserializar y asignar ParentComponent usando AddChild
                if (entityNode["ParentComponent"]) {
                    uint32_t parentID = entityNode["ParentComponent"]["Parent"].as<uint32_t>();
                    if (entityMap.find(parentID) != entityMap.end()) {
                        entt::entity parentEntity = entityMap[parentID];
                        entityManager.AddChild(parentEntity, entity); // Usar la función AddChild del EntityManager
                    }
                }

                // Deserializar y asignar ChildComponent usando AddChild
                if (entityNode["ChildComponent"]) {
                    auto childrenNode = entityNode["ChildComponent"];
                    for (auto childIDNode : childrenNode) {
                        uint32_t childID = childIDNode.as<uint32_t>();
                        if (entityMap.find(childID) != entityMap.end()) {
                            entt::entity childEntity = entityMap[childID];
                            entityManager.AddChild(entity, childEntity); // Usar la función AddChild del EntityManager
                        }
                    }
                }
            }
        }
    }
    //---------------------------------------------------------------------------------------------

    //--SERIALIZAR COMPONENTES
    void Scene::SerializeComponents() 
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << sceneName + "_temp";

        // Serializar solo los componentes de las entidades
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        auto& entityManager = EntityManager::GetInstance();
        entityManager.m_registry->each([&](auto entity) {

            out << YAML::BeginMap; // Entity
            out << YAML::Key << "Entity" << YAML::Value << (uint32_t)entity; // Serialize entity ID

            if (entityManager.HasComponent<IDComponent>(entity)) {
                out << YAML::Key << "IDComponent";
                out << YAML::Value << SerializeIDComponent(entityManager.GetComponent<IDComponent>(entity));
            }

            if (entityManager.HasComponent<TagComponent>(entity)) {
                out << YAML::Key << "TagComponent";
                out << YAML::Value << SerializeTagComponent(entityManager.GetComponent<TagComponent>(entity));
            }

            // Serializar solo los componentes
            if (entityManager.HasComponent<TransformComponent>(entity)) {
                out << YAML::Key << "TransformComponent";
                out << YAML::Value << SerializeTransformComponent(entityManager.GetComponent<TransformComponent>(entity));
            }

            /*if (entityManager.HasComponent<MeshComponent>(entity)) {
                out << YAML::Key << "MeshComponent";
                out << YAML::Value << SerializeMeshComponent(entityManager.GetComponent<MeshComponent>(entity));
            }*/

            //if (entityManager.HasComponent<MaterialComponent>(entity)) {
            //    out << YAML::Key << "MaterialComponent";
            //    out << YAML::Value << SerializeMaterialComponent(entityManager.GetComponent<MaterialComponent>(entity));
            //}

            if (entityManager.HasComponent<ScriptComponent>(entity)) {
                out << YAML::Key << "ScriptComponent";
                out << YAML::Value << SerializeScriptComponent(entityManager.GetComponent<ScriptComponent>(entity));
            }

            out << YAML::EndMap; // Entity
            });

        out << YAML::EndSeq;
        out << YAML::EndMap;

        // Guardar en archivo temporal
        std::ofstream fout("assets/Scenes/" + sceneName + "_temp.yaml");
        fout << out.c_str();
    }
    //---------------------------------------------------------------------------------------------

    //--DESERIALIZAR COMPONENTES
    void Scene::DeserializeComponents() {
        YAML::Node data = YAML::LoadFile("assets/Scenes/" + sceneName + "_temp.yaml");
        if (!data["Scene"]) {
            return;
        }

        auto& entityManager = EntityManager::GetInstance();
        auto entities = data["Entities"];

        // Mapa para mantener la correspondencia entre los IDs de las entidades y las entidades deserializadas
        std::unordered_map<uint32_t, entt::entity> entityMap;

        if (entities) {
            // Deserializar los componentes básicos
            for (auto entityNode : entities) {
                uint32_t entityID = entityNode["Entity"].as<uint32_t>();

                if (entityNode["IDComponent"]) {

                    auto id_component = DeserializeIDComponent(entityNode["IDComponent"]);

                    entt::entity entity = entityManager.GetEntityByUUID(id_component.ID);

                    if (entityNode["TagComponent"]) {
                        auto tag_component = DeserializeTagComponent(entityNode["TagComponent"]);
                        entityManager.m_registry->emplace_or_replace<TagComponent>(entity, tag_component);
                    }
                    if (entityNode["TransformComponent"]) {
                        auto transform_component = DeserializeTransformComponent(entityNode["TransformComponent"]);
                        entityManager.m_registry->emplace_or_replace<TransformComponent>(entity, transform_component);
                    }

                    //if (entityNode["MeshComponent"]) {
                    //    auto mesh_component = DeserializeMeshComponent(entityNode["MeshComponent"]);
                    //    entityManager.m_registry->emplace_or_replace<MeshComponent>(entity, mesh_component);
                    //}

                    //if (entityNode["MaterialComponent"]) {
                    //    auto material_component = DeserializeMaterialComponent(entityNode["MaterialComponent"]);
                    //    entityManager.m_registry->emplace_or_replace<MaterialComponent>(entity, material_component);
                    //}

                    if (entityNode["ScriptComponent"]) {
                        auto script_component = DeserializeScriptComponent(entityNode["ScriptComponent"]);
                        entityManager.m_registry->emplace_or_replace<ScriptComponent>(entity, script_component);
                    }

                    entityMap[entityID] = entity;
                }
            }
        }
    }
    //---------------------------------------------------------------------------------------------
}
