#pragma once
#include "PanelBase.h"
#include "../../managers/ViewportManager.hpp"

namespace libCore
{
    class FrameBuffersPreviewPanel : public PanelBase
    {
    public:
        FrameBuffersPreviewPanel() : PanelBase("Framebuffers"), m_ImageSize(128.0f) {}

        void Init() override {}

        void Draw() override
        {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());

            // Slider para ajustar el tamaño de las imágenes
            ImGui::SliderFloat("Image Size", &m_ImageSize, 64.0f, 512.0f);

            // Obtener el tamaño del panel disponible
            ImVec2 panelSize = ImGui::GetContentRegionAvail();
            int columns = static_cast<int>(panelSize.x / m_ImageSize);

            if (ImGui::CollapsingHeader("GBuffer Textures")) {
                glDisable(GL_DEPTH_TEST); // Deshabilitar test de profundidad

                ImGui::Columns(columns, nullptr, false);

                DrawTexture("Position", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("position"));
                DrawTexture("Depth", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("depth"));
                DrawTexture("Position View", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("positionView"));
                DrawTexture("Normal", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("normal"));
                DrawTexture("Albedo", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("albedo"));
                DrawTexture("Metallic Roughness", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("metallicRoughness"));
                DrawTexture("Specular", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("specular"));

                ImGui::Columns(1);  // Volver a una columna
                glEnable(GL_DEPTH_TEST); // Rehabilitar test de profundidad
            }

            if (ImGui::CollapsingHeader("Framebuffers")) {
                ImGui::Columns(columns, nullptr, false);

                DrawTexture("Shadow Map Texture", ViewportManager::GetInstance().viewports[0]->framebuffer_shadowmap->getTexture("depth"));
                DrawTexture("SSAO Blur Texture", ViewportManager::GetInstance().viewports[0]->framebuffer_SSAOBlur->getTexture("color"));
                DrawTexture("Deferred FBO", ViewportManager::GetInstance().viewports[0]->framebuffer_deferred->getTexture("color"));
                DrawTexture("Forward FBO", ViewportManager::GetInstance().viewports[0]->framebuffer_forward->getTexture("color"));

                ImGui::Columns(1);  // Volver a una columna
            }

            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }

    private:
        float m_ImageSize;

        void DrawTexture(const std::string& label, GLuint textureID) {
            if (textureID == 0) {
                std::cerr << "Error: " << label << " texture is invalid!" << std::endl;
                return;
            }
            ImGui::Text("%s", label.c_str());
            ImGui::Image((void*)(intptr_t)textureID, ImVec2(m_ImageSize, m_ImageSize), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::NextColumn();  // Pasar a la siguiente columna
        }
    };
}
