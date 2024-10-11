#pragma once
#include "PanelBase.h"
#include "../../managers/EntityManager.h"
#include "../../managers/LuaManager.h"


namespace libCore
{
    class InspectorPanel : public PanelBase
    {
    public:
        InspectorPanel() : PanelBase("Inspector") {}
        void Init() override{}
        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            entt::entity selectedEntity = EntityManager::GetInstance().currentSelectedEntityInScene;

            if (selectedEntity != entt::null && EntityManager::GetInstance().m_registry->valid(selectedEntity))
            {
                DrawComponentEditor(selectedEntity);

                //--UUID_COMPONENT
                if (EntityManager::GetInstance().HasComponent<IDComponent>(selectedEntity)) {
                    auto& idComponent = EntityManager::GetInstance().GetComponent<IDComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_ID_BADGE " ID")) {
                        ImGui::Text("UUID: %s", idComponent.ID.ToString().c_str());

                        // Botón para eliminar la entidad
                        if (ImGui::Button(ICON_FA_TRASH " Delete Entity")) {
                            EntityManager::GetInstance().MarkToDeleteRecursively(EntityManager::GetInstance().currentSelectedEntityInScene);
                            EntityManager::GetInstance().currentSelectedEntityInScene = entt::null;
                        }
                    }
                }
                //--CreatedInRunTimeComponent_component
                if (EntityManager::GetInstance().HasComponent<CreatedInRunTimeComponent>(selectedEntity)) {
                    auto& createdInRunTimeComponent = EntityManager::GetInstance().GetComponent<CreatedInRunTimeComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_ID_BADGE "DESTROY_")) 
                    {
                        ImGui::Text("CREATED IN RUN TIME, THIS ENTITY WILL BE DESTORY");
                    }
                }
                //--TAG_COMPONENT
                if (EntityManager::GetInstance().HasComponent<TagComponent>(selectedEntity)) {
                    auto& tagComponent = EntityManager::GetInstance().GetComponent<TagComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_TAG " Tag")) {
                        ImGui::Text("Current Tag: %s", tagComponent.Tag.c_str());
                        static char buffer[256];

                        if (strlen(buffer) == 0) {
                            strncpy_s(buffer, tagComponent.Tag.c_str(), sizeof(buffer));
                            buffer[sizeof(buffer) - 1] = '\0';
                        }

                        if (ImGui::InputText("New Tag", buffer, sizeof(buffer))) {
                        }

                        if (ImGui::Button(ICON_FA_PENCIL_ALT " Update Tag")) {
                            tagComponent.Tag = std::string(buffer);
                            buffer[0] = '\0';
                        }
                    }
                }
                //--PARENT_COMPONENT
                if (EntityManager::GetInstance().HasComponent<ParentComponent>(selectedEntity)) {
                    auto& parentComponent = EntityManager::GetInstance().GetComponent<ParentComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_USER_FRIENDS " Parent")) {
                        std::string parentTag = "None";
                        if (EntityManager::GetInstance().HasComponent<TagComponent>(parentComponent.parent)) {
                            parentTag = EntityManager::GetInstance().GetComponent<TagComponent>(parentComponent.parent).Tag;
                        }
                        ImGui::Text("Parent Entity: %s", parentTag.c_str());
                    }
                }
                //--CHILD_COMPONENT
                if (EntityManager::GetInstance().HasComponent<ChildComponent>(selectedEntity)) {
                    auto& childComponent = EntityManager::GetInstance().GetComponent<ChildComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_USERS " Children")) {
                        for (auto child : childComponent.children) {
                            if (EntityManager::GetInstance().HasComponent<TagComponent>(child)) {
                                std::string childTag = EntityManager::GetInstance().GetComponent<TagComponent>(child).Tag;
                                ImGui::Text("Child Entity: %s", childTag.c_str());
                            }
                        }
                    }
                }
                //--TRANSFORM_COMPONENT
                if (EntityManager::GetInstance().HasComponent<TransformComponent>(selectedEntity)) {
                    auto& transformComponent = EntityManager::GetInstance().GetComponent<TransformComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_ARROWS_ALT " Transform")) {
                        auto& transform = transformComponent.transform;

                        ImGui::Text("Position");
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "X");
                        ImGui::SameLine();
                        ImGui::DragFloat("##PosX", &transform->position.x, 0.1f, -FLT_MAX, FLT_MAX, "X: %.2f");
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Y");
                        ImGui::SameLine();
                        ImGui::DragFloat("##PosY", &transform->position.y, 0.1f, -FLT_MAX, FLT_MAX, "Y: %.2f");
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "Z");
                        ImGui::SameLine();
                        ImGui::DragFloat("##PosZ", &transform->position.z, 0.1f, -FLT_MAX, FLT_MAX, "Z: %.2f");

                        ImGui::Text("Rotation");
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "X");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##RotX", &transform->eulerAngles.x, 0.1f, -360.0f, 360.0f, "X: %.2f")) {
                            transform->updateRotationFromEulerAngles();
                        }
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Y");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##RotY", &transform->eulerAngles.y, 0.1f, -360.0f, 360.0f, "Y: %.2f")) {
                            transform->updateRotationFromEulerAngles();
                        }
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "Z");
                        ImGui::SameLine();
                        if (ImGui::DragFloat("##RotZ", &transform->eulerAngles.z, 0.1f, -360.0f, 360.0f, "Z: %.2f")) {
                            transform->updateRotationFromEulerAngles();
                        }

                        ImGui::Text("Scale");
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "X");
                        ImGui::SameLine();
                        ImGui::DragFloat("##ScaleX", &transform->scale.x, 0.01f, 0.0f, FLT_MAX, "X: %.2f");
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Y");
                        ImGui::SameLine();
                        ImGui::DragFloat("##ScaleY", &transform->scale.y, 0.01f, 0.0f, FLT_MAX, "Y: %.2f");
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "Z");
                        ImGui::SameLine();
                        ImGui::DragFloat("##ScaleZ", &transform->scale.z, 0.01f, 0.0f, FLT_MAX, "Z: %.2f");
                    }
                }
                //--MESH_COMPONENT
                if (EntityManager::GetInstance().HasComponent<MeshComponent>(selectedEntity)) {
                    if (ImGui::CollapsingHeader(ICON_FA_CUBES " Mesh")) {
                        auto& meshComponent = EntityManager::GetInstance().GetComponent<MeshComponent>(selectedEntity);

                        // Mostrar nombre de la mesh e información básica
                        ImGui::Text("Mesh Name: %s", meshComponent.mesh->meshName.c_str());
                        ImGui::Text("Instance: %s", meshComponent.isInstance ? "Yes" : "No");
                        ImGui::Checkbox("Renderable", &meshComponent.renderable);

                        // Mostrar huesos que afectan a esta Mesh
                        if (ImGui::TreeNode("Influencing Bones")) {
                            auto& boneInfoMap = meshComponent.originalModel->GetBoneInfoMap();
                            for (int boneID : meshComponent.mesh->influencingBones) {
                                // Busca el nombre del hueso en el mapa de huesos
                                for (const auto& [boneName, boneInfo] : boneInfoMap) {
                                    if (boneInfo.id == boneID) {
                                        ImGui::Text("Bone Name: %s (Bone ID: %d)", boneName.c_str(), boneID);

                                        // Contar los vértices afectados por este hueso
                                        int affectedVerticesCount = 0;
                                        for (const Vertex& vertex : meshComponent.mesh->vertices) {
                                            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                                                if (vertex.m_BoneIDs[i] == boneID) {
                                                    ++affectedVerticesCount;
                                                    break;
                                                }
                                            }
                                        }
                                        ImGui::Text("Affected Vertices: %d", affectedVerticesCount);
                                        break;
                                    }
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                }


                ////--MESH_COMPONENT
                //if (EntityManager::GetInstance().HasComponent<MeshComponent>(selectedEntity)) {
                //    if (ImGui::CollapsingHeader(ICON_FA_CUBES " Mesh")) {
                //        auto& meshComponent = EntityManager::GetInstance().GetComponent<MeshComponent>(selectedEntity);
                //        ImGui::Text("Mesh Name: %s", meshComponent.mesh->meshName.c_str());
                //        ImGui::Text("Instance: %s", meshComponent.isInstance ? "Yes" : "No");
                //        ImGui::Checkbox("Renderable", &meshComponent.renderable);
                //        
                //    }
                //}
                //--ANIMATION_COMPONENT
                if (EntityManager::GetInstance().HasComponent<AnimationComponent>(selectedEntity)) {
                    if (ImGui::CollapsingHeader(ICON_FA_FILM " Animation")) {  // Icono de "película" para el componente de animación
                        auto& animationComponent = EntityManager::GetInstance().GetComponent<AnimationComponent>(selectedEntity);
                        ImGui::Separator();

                        // Campo para hacer drag & drop de una animación
                        static char animationPath[128] = "";
                        ImGui::InputText("Animation Path", animationPath, IM_ARRAYSIZE(animationPath));

                        // Aquí permitimos que el usuario haga drop de una animación
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAG")) {
                                const char* droppedPath = (const char*)payload->Data;
                                strncpy(animationPath, droppedPath, IM_ARRAYSIZE(animationPath));
                            }
                            ImGui::EndDragDropTarget();
                        }

                        ImGui::Separator();

                        ImGui::SliderFloat("Playback Speed", &animationComponent.playbackSpeed, 0.1f, 3.0f);
                        ImGui::SliderFloat("Bone Scale Factor", &animationComponent.boneScaleFactor, 0.01f, 200.0f);

                        if (ImGui::Button("Add Animation")) {
                            if (strlen(animationPath) > 0) {
                                std::string name = "Animation_" + std::to_string(animationComponent.animations.size());
                                animationComponent.AddAnimation(name, animationPath);
                            }
                        }

                        ImGui::Separator();

                        // Mostrar lista de animaciones cargadas con botones Play, Pause y Stop con iconos
                        if (!animationComponent.animations.empty()) {
                            for (auto& [name, animation] : animationComponent.animations) {
                                // Mostrar el nombre de la animación
                                ImGui::Text("%s", name.c_str());

                                // Dependiendo del estado de la animación, mostramos los botones correspondientes
                                if (animationComponent.currentAnimation == name && animationComponent.isPlaying) {
                                    // Si la animación está en reproducción, mostramos los botones Pause y Stop
                                    ImGui::SameLine();
                                    if (ImGui::Button(ICON_FA_PAUSE "##Pause")) {
                                        animationComponent.isPlaying = false;  // Pausar la animación
                                    }

                                    ImGui::SameLine();
                                    if (ImGui::Button(ICON_FA_STOP "##Stop")) {
                                        animationComponent.isPlaying = false;  // Detener la animación
                                        animationComponent.animationTime = 0.0f;  // Reiniciar el tiempo de la animación
                                    }
                                }
                                else {
                                    // Si la animación está parada, mostramos solo el botón Play
                                    ImGui::SameLine();
                                    if (ImGui::Button(ICON_FA_PLAY "##Play")) {
                                        animationComponent.SetCurrentAnimation(name);
                                        animationComponent.isPlaying = true;  // Reproducir la animación
                                    }
                                }
                            }
                        }
                    }
                }
                //--AABB_COMPONENT
                if (EntityManager::GetInstance().HasComponent<AABBComponent>(selectedEntity)) {
                    if (ImGui::CollapsingHeader(ICON_FA_CUBE " AABB")) {  // Añadir icono de cubo para el AABB
                        auto& aabbComponent = EntityManager::GetInstance().GetComponent<AABBComponent>(selectedEntity);
                        ImGui::Checkbox(ICON_FA_EYE " Show AABB", &aabbComponent.aabb->showAABB);  // Icono de ojo para mostrar/ocultar AABB
                    }
                }
                //--MATERIAL_COMPONENT
                if (EntityManager::GetInstance().HasComponent<MaterialComponent>(selectedEntity)) {
                    auto& materialComponent = EntityManager::GetInstance().GetComponent<MaterialComponent>(selectedEntity);
                    auto& material = *materialComponent.material;

                    if (ImGui::CollapsingHeader(ICON_FA_PALETTE " Material", ImGuiTreeNodeFlags_DefaultOpen)) {  // Icono de paleta para el material
                        ImGui::Text(ICON_FA_TAG " Material Name: %s", material.materialName.c_str());  // Icono de etiqueta para el nombre del material

                        // Drag Target para el material completo
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL")) {
                                IM_ASSERT(payload->DataSize == sizeof(Ref<Material>));
                                Ref<Material> newMaterial = *(const Ref<Material>*)payload->Data;
                                materialComponent.material = newMaterial;
                            }
                            ImGui::EndDragDropTarget();
                        }

                        //ImGui::Text(ICON_FA_CODE " Shader Name: %s", material.shaderName.c_str());  // Icono de código para el shader

                        ImGui::ColorEdit3(ICON_FA_PAINT_BRUSH " Albedo Color", (float*)&materialComponent.material->albedoColor);
                        ImGui::DragFloat(ICON_FA_ADJUST " Normal Strength", &materialComponent.material->normalStrength, 0.1f, -10.0f, 10.0f);
                        ImGui::DragFloat(ICON_FA_COG " Metallic Value", &materialComponent.material->metallicValue, 0.1f, 0.0f, 10.0f);
                        ImGui::DragFloat(ICON_FA_RULER " Roughness Value", &materialComponent.material->roughnessValue, 0.1f, 0.0f, 10.0f);

                        auto drawTextureSlot = [](const char* label, Ref<Texture>& texture) {
                            ImGui::Text("%s", label);
                            if (texture && texture->IsValid()) {
                                ImGui::Image((void*)(intptr_t)texture->GetTextureID(), ImVec2(128, 128));
                            }
                            else {
                                ImGui::Text("None");
                            }

                            if (ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE")) {
                                    IM_ASSERT(payload->DataSize == sizeof(Ref<Texture>));
                                    texture = *(const Ref<Texture>*)payload->Data;
                                }
                                ImGui::EndDragDropTarget();
                            }
                        };

                        drawTextureSlot(ICON_FA_IMAGE " Albedo Map", materialComponent.material->albedoMap);
                        drawTextureSlot(ICON_FA_IMAGE " Normal Map", materialComponent.material->normalMap);
                        drawTextureSlot(ICON_FA_IMAGE " Metallic Map", materialComponent.material->metallicMap);
                        drawTextureSlot(ICON_FA_IMAGE " Roughness Map", materialComponent.material->roughnessMap);
                    }
                }
                //--CAMERA_COMPONENT
                if (EntityManager::GetInstance().HasComponent<CameraComponent>(selectedEntity)) {
                    auto& cameraComponent = EntityManager::GetInstance().GetComponent<CameraComponent>(selectedEntity);
                    if (ImGui::CollapsingHeader(ICON_FA_CAMERA " Camera"))  // Se añade el ícono de cámara
                    {
                        ImGui::Text("Width: %d", cameraComponent.camera->width);
                        ImGui::Text("Height: %d", cameraComponent.camera->height);

                        ImGui::Text(ICON_FA_ARROWS_ALT " Up Vector");  // Ícono de vectores
                        ImGui::DragFloat3("Up", glm::value_ptr(cameraComponent.camera->Up), 0.1f);

                        ImGui::Text(ICON_FA_EXPAND_ARROWS_ALT " FOV: %.2f", cameraComponent.camera->FOVdeg);  // Ícono de FOV
                        ImGui::DragFloat("FOV", &cameraComponent.camera->FOVdeg, 0.1f, 1.0f, 180.0f);

                        ImGui::Text(ICON_FA_MAP_MARKED_ALT " Near Plane: %.2f", cameraComponent.camera->nearPlane);  // Ícono de plano cercano
                        ImGui::DragFloat("Near Plane", &cameraComponent.camera->nearPlane, 0.01f, 0.01f, cameraComponent.camera->farPlane - 0.1f);

                        ImGui::Text(ICON_FA_MAP_MARKED_ALT " Far Plane: %.2f", cameraComponent.camera->farPlane);  // Ícono de plano lejano
                        ImGui::DragFloat("Far Plane", &cameraComponent.camera->farPlane, 1.0f, cameraComponent.camera->nearPlane + 0.1f, 10000.0f);

                        ImGui::Text(ICON_FA_TH " View Matrix");  // Ícono de matriz de vista
                        ImGui::InputFloat4("##ViewRow1", glm::value_ptr(cameraComponent.camera->view[0]));
                        ImGui::InputFloat4("##ViewRow2", glm::value_ptr(cameraComponent.camera->view[1]));
                        ImGui::InputFloat4("##ViewRow3", glm::value_ptr(cameraComponent.camera->view[2]));
                        ImGui::InputFloat4("##ViewRow4", glm::value_ptr(cameraComponent.camera->view[3]));

                        ImGui::Text(ICON_FA_TH_LARGE " Projection Matrix");  // Ícono de matriz de proyección
                        ImGui::InputFloat4("##ProjRow1", glm::value_ptr(cameraComponent.camera->projection[0]));
                        ImGui::InputFloat4("##ProjRow2", glm::value_ptr(cameraComponent.camera->projection[1]));
                        ImGui::InputFloat4("##ProjRow3", glm::value_ptr(cameraComponent.camera->projection[2]));
                        ImGui::InputFloat4("##ProjRow4", glm::value_ptr(cameraComponent.camera->projection[3]));

                        ImGui::Text(ICON_FA_TH_LARGE " Camera Matrix");  // Ícono de matriz de cámara
                        ImGui::InputFloat4("##CamRow1", glm::value_ptr(cameraComponent.camera->cameraMatrix[0]));
                        ImGui::InputFloat4("##CamRow2", glm::value_ptr(cameraComponent.camera->cameraMatrix[1]));
                        ImGui::InputFloat4("##CamRow3", glm::value_ptr(cameraComponent.camera->cameraMatrix[2]));
                        ImGui::InputFloat4("##CamRow4", glm::value_ptr(cameraComponent.camera->cameraMatrix[3]));
                    }
                }
                //--SCRIPT_COMPONENT
                if (EntityManager::GetInstance().HasComponent<ScriptComponent>(selectedEntity)) {
                    auto& scriptComponent = EntityManager::GetInstance().GetComponent<ScriptComponent>(selectedEntity);

                    if (ImGui::CollapsingHeader(ICON_FA_CODE " Scripts", ImGuiTreeNodeFlags_DefaultOpen))  // Ícono de código
                    {
                        // Mostrar todos los scripts asignados
                        const auto& luaScripts = scriptComponent.GetLuaScriptsData();
                        
                        for (const auto& scriptData : luaScripts) {
                            ImGui::Text(ICON_FA_FILE_CODE " Assigned Script: %s", scriptData.name.c_str());  // Ícono de archivo de script

                            if (ImGui::Button((ICON_FA_SYNC_ALT " Reload " + scriptData.name).c_str())) {  // Ícono de recargar
                                LuaManager::GetInstance().ReloadLuaFile(scriptData.name);
                            }

                            sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptData.name);
                            sol::table exposedVars = lua["exposedVars"];

                            if (!exposedVars.valid()) {
                                ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE " No exposed variables found");  // Ícono de advertencia
                                continue;
                            }

                            // Mostrar variables expuestas
                            for (auto& kvp : exposedVars) {
                                std::string varName = kvp.first.as<std::string>();
                                sol::object varValue = kvp.second;

                                if (varName.rfind("int_", 0) == 0) {
                                    int value = static_cast<int>(varValue.as<float>());
                                    if (ImGui::SliderInt(varName.c_str(), &value, 0, 100)) {
                                        exposedVars[varName] = value;
                                    }
                                }
                                else if (varValue.is<float>()) {
                                    float value = varValue.as<float>();
                                    if (ImGui::SliderFloat(varName.c_str(), &value, 0.0f, 10.0f)) {
                                        exposedVars[varName] = value;
                                    }
                                }
                                else if (varValue.is<bool>()) {
                                    bool value = varValue.as<bool>();
                                    if (ImGui::Checkbox(varName.c_str(), &value)) {
                                        exposedVars[varName] = value;
                                    }
                                }
                                else if (varName == "selectedModel" && varValue.is<std::string>()) {
                                    char buffer[256];
                                    strncpy_s(buffer, sizeof(buffer), varValue.as<std::string>().c_str(), _TRUNCATE);
                                    if (ImGui::InputText(varName.c_str(), buffer, sizeof(buffer))) {
                                        exposedVars[varName] = std::string(buffer);
                                    }
                                    if (ImGui::BeginDragDropTarget()) {
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_PAYLOAD")) {
                                            const char* droppedModelName = (const char*)payload->Data;
                                            strncpy_s(buffer, sizeof(buffer), droppedModelName, _TRUNCATE);
                                            exposedVars[varName] = std::string(buffer);
                                        }
                                        ImGui::EndDragDropTarget();
                                    }
                                }
                                else if (varValue.is<sol::table>()) {
                                    sol::table vectorTable = varValue.as<sol::table>();
                                    if (vectorTable.size() == 3) {
                                        float vec3[3];
                                        vec3[0] = vectorTable[1].get<float>();
                                        vec3[1] = vectorTable[2].get<float>();
                                        vec3[2] = vectorTable[3].get<float>();
                                        if (ImGui::InputFloat3(varName.c_str(), vec3)) {
                                            vectorTable[1] = vec3[0];
                                            vectorTable[2] = vec3[1];
                                            vectorTable[3] = vec3[2];
                                        }
                                    }
                                }
                            }

                            if (ImGui::Button((ICON_FA_TRASH_ALT " Remove " + scriptData.name).c_str())) {  // Ícono de eliminación
                                scriptComponent.RemoveLuaScript(scriptData.name);
                            }
                        }

                        const auto loadedScripts = LuaManager::GetInstance().GetLoadedScripts();
                        static std::string selectedScript;
                        std::vector<ImportLUA_ScriptData> scriptDataList;

                        for (const auto& scriptData : loadedScripts) {
                            scriptDataList.push_back(scriptData);
                        }

                        if (ImGui::BeginCombo(ICON_FA_PLUS " Add Script", selectedScript.c_str())) {  // Ícono de añadir
                            for (const auto& scriptData : scriptDataList) {
                                bool isSelected = (selectedScript == scriptData.name);
                                if (ImGui::Selectable(scriptData.name.c_str(), isSelected)) {
                                    selectedScript = scriptData.name;
                                }
                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        if (ImGui::Button(ICON_FA_PLUS " Assign Script")) {  // Botón de asignación de script con ícono de añadir
                            auto it = std::find_if(scriptDataList.begin(), scriptDataList.end(),
                                [&](const ImportLUA_ScriptData& data) { return data.name == selectedScript; });

                            if (it != scriptDataList.end()) {
                                scriptComponent.AddLuaScript(*it);
                            }

                            selectedScript.clear();  // Limpiar la selección después de asignar
                        }
                    }
                }
            }
            else {
                ImGui::Text("No entity selected.");
            }
            //--
            ImGui::End();
        }
        void DrawComponentEditor(entt::entity entity) {
            auto& entityManager = EntityManager::GetInstance();

            static std::string selectedComponent;

            // Lista de todos los componentes disponibles con sus iconos correspondientes
            std::vector<std::pair<std::string, const char*>> componentNames = {
                {"TransformComponent", ICON_FA_ARROWS_ALT},        // Icono para transformación
                {"MeshComponent",      ICON_FA_CUBE},              // Icono para mallas
                {"MaterialComponent",  ICON_FA_PAINT_BRUSH},       // Icono para material
                {"CameraComponent",    ICON_FA_CAMERA},            // Icono para cámara
                {"LightComponent",     ICON_FA_LIGHTBULB},         // Icono para luz
                {"ScriptComponent",    ICON_FA_CODE}               // Icono para scripts
            };

            // ComboBox para seleccionar el componente
            if (ImGui::BeginCombo("Components", selectedComponent.c_str())) {
                for (const auto& component : componentNames) {
                    bool isSelected = (selectedComponent == component.first);
                    if (ImGui::Selectable((std::string(component.second) + " " + component.first).c_str(), isSelected)) {
                        selectedComponent = component.first;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Botón para agregar el componente a la entidad
            if (ImGui::Button(ICON_FA_PLUS " Add Component") && !selectedComponent.empty()) {
                if (selectedComponent == "TransformComponent") {
                    entityManager.AddComponent<TransformComponent>(entity);
                }
                else if (selectedComponent == "MeshComponent") {
                    entityManager.AddComponent<MeshComponent>(entity);
                }
                else if (selectedComponent == "MaterialComponent") {
                    entityManager.AddComponent<MaterialComponent>(entity);
                }
                else if (selectedComponent == "CameraComponent") {
                    entityManager.AddComponent<CameraComponent>(entity);
                }
                /*else if (selectedComponent == "LightComponent") {
                    entityManager.AddComponent<LightComponent>(entity);
                }
                else if (selectedComponent == "DirectionalLightComponent") {
                    entityManager.AddComponent<DirectionalLightComponent>(entity);
                }*/
                else if (selectedComponent == "ScriptComponent") {
                    entityManager.AddComponent<ScriptComponent>(entity);
                }
                selectedComponent.clear(); // Limpiar la selección después de agregar el componente
            }
        }
        void Shutdown() override {}
    };
}