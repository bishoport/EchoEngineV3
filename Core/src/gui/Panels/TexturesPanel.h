#pragma once
#include "PanelBase.h"

#include "../../managers/AssetsManager.h"

namespace libCore
{
    class TexturesPanel : public PanelBase
    {
    public:
        TexturesPanel() : PanelBase("Textures") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            auto& assetsManager = libCore::AssetsManager::GetInstance();
            const auto& textures = assetsManager.GetAllTextures();
            std::size_t numberOfTextures = assetsManager.GetNumberOfTextures();

            static float imageSize = 64.0f; // Tamaño inicial de la imagen
            static float cellPadding = 0.0f; // Padding inicial entre celdas

            ImGui::Begin(m_title.c_str());
            //--

            ImGui::Spacing();

            // Show texture count with an icon
            ImGui::Text(ICON_FA_IMAGES " Texture Count: %zu", numberOfTextures);

            ImGui::Spacing();
            ImGui::SliderFloat(ICON_FA_EXPAND_ARROWS_ALT " Image Size", &imageSize, 64.0f, 256.0f);
            ImGui::Spacing();

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columns = static_cast<int>(panelWidth / (imageSize + cellPadding));
            if (columns < 1) columns = 1;

            ImGui::Columns(columns, nullptr, false);

            for (const auto& texturePair : textures) {
                const auto& textureName = texturePair.first;
                const auto& texture = texturePair.second;

                if (texture != nullptr && texture->IsValid()) {
                    ImGui::PushID(textureName.c_str());

                    // Show the image with a drag-and-drop option
                    ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture->GetTextureID())), ImVec2(imageSize, imageSize));
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("TEXTURE", &texture, sizeof(Ref<Texture>));
                        ImGui::Text(ICON_FA_HAND_ROCK "%s", textureName.c_str());
                        ImGui::EndDragDropSource();
                    }

                    ImGui::TextWrapped(ICON_FA_TAG "%s", textureName.c_str());
                    //ImGui::TextWrapped(ICON_FA_OBJECT_GROUP " Slot: %u", texture->m_unit);

                    ImGui::Dummy(ImVec2(0.0f, cellPadding));
                    ImGui::NextColumn();

                    ImGui::PopID();
                }
                else {
                    ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE " Invalid texture: %s", textureName.c_str());
                }
            }

            ImGui::Columns(1);
            ImGui::End();
        }


        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}
