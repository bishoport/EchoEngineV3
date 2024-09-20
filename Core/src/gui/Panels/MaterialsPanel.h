#pragma once
#include "PanelBase.h"

#include "../../managers/AssetsManager.h"

namespace libCore
{
    class MaterialsPanel : public PanelBase
    {
    public:
        MaterialsPanel() : PanelBase("Materials") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            // Añadir un control deslizante para ajustar el tamaño de la previsualización
            float previewSize = 64.0f; // Tamaño inicial de la previsualización
            float cellPadding = 0.0f;  // Padding inicial entre celdas
            ImGui::SliderFloat(ICON_FA_EXPAND " Preview Size", &previewSize, 32.0f, 256.0f);

            for (auto& pair : AssetsManager::GetInstance().GetAllMaterials()) {
                std::string key = pair.first;
                Ref<Material> material = pair.second;

                if (key.empty()) {
                    key = "Unnamed Material";
                }

                std::string nodeId = key + "##" + key;

                if (ImGui::TreeNode(nodeId.c_str(), ICON_FA_PAINT_BRUSH " Material: %s", material->materialName.c_str())) {

                    ImGui::Spacing();

                    //-Drag Source
                    ImGui::Text(ICON_FA_TAG " Material: %s", material->materialName.c_str());
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("MATERIAL", &material, sizeof(Ref<Material>)); // Set payload
                        ImGui::Text(ICON_FA_HAND_HOLDING " Drag Material: %s", material->materialName.c_str());
                        ImGui::EndDragDropSource();
                    }
                    //----------------------

                    ImGui::Spacing();
                    ImGui::Spacing();

                    ImGui::ColorEdit3(ICON_FA_TINT " Albedo Color", (float*)&material->albedoColor);
                    ImGui::DragFloat(ICON_FA_BRUSH " Normal Strength", &material->normalStrength, 0.1f, -10.0f, 10.0f);
                    ImGui::DragFloat(ICON_FA_GEM " Metallic Value", &material->metallicValue, 0.1f, 0.0f, 10.0f);
                    ImGui::DragFloat(ICON_FA_FILE " Roughness Value", &material->roughnessValue, 0.1f, 0.0f, 10.0f);

                    // Calcular el número de columnas basado en el tamaño de la imagen y el padding
                    float panelWidth = ImGui::GetContentRegionAvail().x;
                    int columns = static_cast<int>(panelWidth / (previewSize + cellPadding));
                    if (columns < 1) columns = 1;

                    ImGui::Columns(columns, nullptr, false);

                    // Mostrar texturas del material y permitir arrastrar y soltar
                    auto drawTextureSlot = [previewSize, cellPadding](const char* label, Ref<Texture>& texture) {
                        ImGui::BeginGroup(); // Comienza un grupo para que el nombre y la imagen estén juntos
                        ImGui::Text("%s", label);
                        if (texture && texture->IsValid()) {
                            ImGui::Image((void*)(intptr_t)texture->GetTextureID(), ImVec2(previewSize, previewSize));
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
                        ImGui::EndGroup(); // Termina el grupo
                        ImGui::SameLine(); // Coloca el siguiente elemento en la misma línea
                        ImGui::Dummy(ImVec2(0.0f, cellPadding)); // Añadir espacio extra entre celdas
                        ImGui::NextColumn();
                    };

                    drawTextureSlot(ICON_FA_IMAGE " Albedo Map", material->albedoMap);
                    drawTextureSlot(ICON_FA_IMAGE " Normal Map", material->normalMap);
                    drawTextureSlot(ICON_FA_IMAGE " Metallic Map", material->metallicMap);
                    drawTextureSlot(ICON_FA_IMAGE " Roughness Map", material->roughnessMap);

                    ImGui::Columns(1); // Volver a una sola columna
                    ImGui::NewLine(); // Asegura que el siguiente elemento se coloque en una nueva línea

                    ImGui::TreePop();
                }

                ImGui::Separator();
            }

            //--
            ImGui::End();
        }


        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}