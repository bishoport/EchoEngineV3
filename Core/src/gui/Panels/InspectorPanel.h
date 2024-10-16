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

        void Init() override
        {

        }
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
                        std::string uuidStr = std::to_string(static_cast<uint64_t>(idComponent.ID));
                        ImGui::Text("UUID: %s", uuidStr.c_str());
                        ImGui::Checkbox("Mark To Delete", &idComponent.markToDelete);
                        // Bot�n para eliminar la entidad
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
                        ImGui::Text("Mesh Name: %s", meshComponent.mesh->meshName.c_str());
                        ImGui::Text("Instance: %s", meshComponent.isInstance ? "Yes" : "No");
                        ImGui::Checkbox("Renderable", &meshComponent.renderable);
                        ImGui::Spacing();
                        ImGui::Spacing();
                        // Mostrar informaci�n sobre huesos si existen
                        if (meshComponent.originalModel && !meshComponent.originalModel->GetBoneInfoMap().empty()) {
                            if (ImGui::CollapsingHeader("Bone Information")) {
                                ImGui::Text("Number of Bones: %d", meshComponent.originalModel->GetBoneCount());

                                // Listar huesos
                                for (const auto& [boneName, boneInfo] : meshComponent.originalModel->GetBoneInfoMap()) {
                                    ImGui::Text("Bone: %s, ID: %d", boneName.c_str(), boneInfo.id);
                                }
                            }
                        }
                    }
                }
                //--ANIMATION_COMPONENT
                if (EntityManager::GetInstance().HasComponent<AnimationComponent>(selectedEntity)) {
                    if (ImGui::CollapsingHeader(ICON_FA_FILM " Animation")) {  // Icono de "pel�cula" para el componente de animaci�n
                        auto& animationComponent = EntityManager::GetInstance().GetComponent<AnimationComponent>(selectedEntity);

                        //// Mostrar la animaci�n actual
                        //ImGui::Text(ICON_FA_PLAY " Current Animation: %s", animationComponent.currentAnimation.empty() ? "None" : animationComponent.currentAnimation.c_str());  // Icono de "reproducci�n"
                        //ImGui::SliderFloat(ICON_FA_TACHOMETER_ALT " Playback Speed", &animationComponent.playbackSpeed, 0.1f, 3.0f, "%.1f");  // Icono de "velocidad"
                        //ImGui::Checkbox(ICON_FA_PLAY_CIRCLE " Playing", &animationComponent.isPlaying);  // Icono de "c�rculo de reproducci�n"

                        //// Si existe una animaci�n actual
                        //if (auto currentAnim = animationComponent.GetCurrentAnimation()) {
                        //    ImGui::Text(ICON_FA_CLOCK " Animation Time: %.2f", animationComponent.animationTime);  // Icono de "reloj"
                        //    ImGui::Text(ICON_FA_HOURGLASS_HALF " Duration: %.2f", currentAnim->GetDuration());  // Icono de "duraci�n"
                        //    ImGui::Text(ICON_FA_CLOCK " Ticks Per Second: %.2f", currentAnim->GetTicksPerSecond());  // Icono de "metr�nomo"

                        //    // Mostrar huesos involucrados en la animaci�n actual
                        //    if (ImGui::CollapsingHeader(ICON_FA_BONE " Bones Information")) {  // Icono de "hueso"
                        //        const auto& boneMap = currentAnim->GetBoneIDMap();
                        //        const auto& finalBoneMatrices = animationComponent.GetFinalBoneMatrices();
                        //        ImGui::Text(ICON_FA_CALENDAR " Number of Bones: %d", (int)boneMap.size());  // Icono de "calendario" para contar

                        //        int boneIndex = 0;
                        //        for (const auto& [boneName, boneInfo] : boneMap) {
                        //            ImGui::Text(ICON_FA_BONE " Bone: %s, ID: %d", boneName.c_str(), boneInfo.id);  // Icono de "hueso" para cada hueso

                        //            if (boneIndex < finalBoneMatrices.size()) {
                        //                // Extraer la posici�n del hueso a partir de la matriz final
                        //                glm::vec3 bonePosition = glm::vec3(finalBoneMatrices[boneIndex][3]);

                        //                // Mostrar la posici�n del hueso en el panel
                        //                ImGui::Text("Position: X: %.2f, Y: %.2f, Z: %.2f", bonePosition.x, bonePosition.y, bonePosition.z);
                        //            }
                        //            boneIndex++;
                        //        }
                        //    }
                        //}

                        ImGui::Spacing();
                        ImGui::Separator();

                        //// Mostrar y seleccionar las animaciones disponibles
                        //if (!animationComponent.animations.empty()) {
                        //    if (ImGui::CollapsingHeader(ICON_FA_LIST " Available Animations")) {  // Icono de "lista"
                        //        for (const auto& [animationName, animRef] : animationComponent.animations) {
                        //            if (ImGui::Selectable((ICON_FA_FILM " " + animationName).c_str(), animationComponent.currentAnimation == animationName)) {
                        //                // Cambiar la animaci�n actual
                        //                animationComponent.SetCurrentAnimation(animationName);
                        //            }
                        //        }
                        //    }
                        //}
                    }
                }





                //--AABB_COMPONENT
                if (EntityManager::GetInstance().HasComponent<AABBComponent>(selectedEntity)) {
                    if (ImGui::CollapsingHeader(ICON_FA_CUBE " AABB")) {  // A�adir icono de cubo para el AABB
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

                        //ImGui::Text(ICON_FA_CODE " Shader Name: %s", material.shaderName.c_str());  // Icono de c�digo para el shader

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
                    if (ImGui::CollapsingHeader(ICON_FA_CAMERA " Camera"))  // Se a�ade el �cono de c�mara
                    {
                        ImGui::Text("Width: %d", cameraComponent.camera->width);
                        ImGui::Text("Height: %d", cameraComponent.camera->height);

                        ImGui::Text(ICON_FA_ARROWS_ALT " Up Vector");  // �cono de vectores
                        ImGui::DragFloat3("Up", glm::value_ptr(cameraComponent.camera->Up), 0.1f);

                        ImGui::Text(ICON_FA_EXPAND_ARROWS_ALT " FOV: %.2f", cameraComponent.camera->FOVdeg);  // �cono de FOV
                        ImGui::DragFloat("FOV", &cameraComponent.camera->FOVdeg, 0.1f, 1.0f, 180.0f);

                        ImGui::Text(ICON_FA_MAP_MARKED_ALT " Near Plane: %.2f", cameraComponent.camera->nearPlane);  // �cono de plano cercano
                        ImGui::DragFloat("Near Plane", &cameraComponent.camera->nearPlane, 0.01f, 0.01f, cameraComponent.camera->farPlane - 0.1f);

                        ImGui::Text(ICON_FA_MAP_MARKED_ALT " Far Plane: %.2f", cameraComponent.camera->farPlane);  // �cono de plano lejano
                        ImGui::DragFloat("Far Plane", &cameraComponent.camera->farPlane, 1.0f, cameraComponent.camera->nearPlane + 0.1f, 10000.0f);

                        ImGui::Text(ICON_FA_TH " View Matrix");  // �cono de matriz de vista
                        ImGui::InputFloat4("##ViewRow1", glm::value_ptr(cameraComponent.camera->view[0]));
                        ImGui::InputFloat4("##ViewRow2", glm::value_ptr(cameraComponent.camera->view[1]));
                        ImGui::InputFloat4("##ViewRow3", glm::value_ptr(cameraComponent.camera->view[2]));
                        ImGui::InputFloat4("##ViewRow4", glm::value_ptr(cameraComponent.camera->view[3]));

                        ImGui::Text(ICON_FA_TH_LARGE " Projection Matrix");  // �cono de matriz de proyecci�n
                        ImGui::InputFloat4("##ProjRow1", glm::value_ptr(cameraComponent.camera->projection[0]));
                        ImGui::InputFloat4("##ProjRow2", glm::value_ptr(cameraComponent.camera->projection[1]));
                        ImGui::InputFloat4("##ProjRow3", glm::value_ptr(cameraComponent.camera->projection[2]));
                        ImGui::InputFloat4("##ProjRow4", glm::value_ptr(cameraComponent.camera->projection[3]));

                        ImGui::Text(ICON_FA_TH_LARGE " Camera Matrix");  // �cono de matriz de c�mara
                        ImGui::InputFloat4("##CamRow1", glm::value_ptr(cameraComponent.camera->cameraMatrix[0]));
                        ImGui::InputFloat4("##CamRow2", glm::value_ptr(cameraComponent.camera->cameraMatrix[1]));
                        ImGui::InputFloat4("##CamRow3", glm::value_ptr(cameraComponent.camera->cameraMatrix[2]));
                        ImGui::InputFloat4("##CamRow4", glm::value_ptr(cameraComponent.camera->cameraMatrix[3]));
                    }
                }
                //--SCRIPT_COMPONENT
                if (EntityManager::GetInstance().HasComponent<ScriptComponent>(selectedEntity)) {
                    auto& scriptComponent = EntityManager::GetInstance().GetComponent<ScriptComponent>(selectedEntity);

                    if (ImGui::CollapsingHeader(ICON_FA_CODE " Scripts", ImGuiTreeNodeFlags_DefaultOpen))  // �cono de c�digo
                    {
                        // Mostrar todos los scripts asignados
                        const auto& luaScripts = scriptComponent.GetLuaScriptsData();
                        for (const auto& scriptData : luaScripts) {
                            ImGui::Text(ICON_FA_FILE_CODE " Assigned Script: %s", scriptData.name.c_str());  // �cono de archivo de script

                            if (ImGui::Button((ICON_FA_SYNC_ALT " Reload " + scriptData.name).c_str())) {  // �cono de recargar
                                LuaManager::GetInstance().ReloadLuaFile(scriptData.name);
                            }

                            sol::state& lua = LuaManager::GetInstance().GetLuaState(scriptData.name);
                            sol::table exposedVars = lua["exposedVars"];

                            if (!exposedVars.valid()) {
                                ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE " No exposed variables found");  // �cono de advertencia
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

                            if (ImGui::Button((ICON_FA_TRASH_ALT " Remove " + scriptData.name).c_str())) {  // �cono de eliminaci�n
                                scriptComponent.RemoveLuaScript(scriptData.name);
                            }
                        }

                        const auto loadedScripts = LuaManager::GetInstance().GetLoadedScripts();
                        static std::string selectedScript;
                        std::vector<ImportLUA_ScriptData> scriptDataList;

                        for (const auto& scriptData : loadedScripts) {
                            scriptDataList.push_back(scriptData);
                        }

                        if (ImGui::BeginCombo(ICON_FA_PLUS " Add Script", selectedScript.c_str())) {  // �cono de a�adir
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

                        if (ImGui::Button(ICON_FA_PLUS " Assign Script")) {  // Bot�n de asignaci�n de script con �cono de a�adir
                            auto it = std::find_if(scriptDataList.begin(), scriptDataList.end(),
                                [&](const ImportLUA_ScriptData& data) { return data.name == selectedScript; });

                            if (it != scriptDataList.end()) {
                                scriptComponent.AddLuaScript(*it);
                            }

                            selectedScript.clear();  // Limpiar la selecci�n despu�s de asignar
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
                {"TransformComponent", ICON_FA_ARROWS_ALT},        // �cono para transformaci�n
                {"MeshComponent", ICON_FA_CUBE},                   // �cono para mallas
                {"MaterialComponent", ICON_FA_PAINT_BRUSH},        // �cono para material
                {"CameraComponent", ICON_FA_CAMERA},               // �cono para c�mara
                {"LightComponent", ICON_FA_LIGHTBULB},             // �cono para luz
                {"DirectionalLightComponent", ICON_FA_SUN},        // �cono para luz direccional
                {"ScriptComponent", ICON_FA_CODE}                  // �cono para scripts
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

            // Bot�n para agregar el componente a la entidad
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
                selectedComponent.clear(); // Limpiar la selecci�n despu�s de agregar el componente
            }
        }

        void Shutdown() override {
            // Liberaci�n de recursos si es necesario
        }
    };
}