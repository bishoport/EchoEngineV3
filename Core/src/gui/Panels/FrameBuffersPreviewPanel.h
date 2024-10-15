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
            int columns = static_cast<int>(panelSize.x / m_ImageSize);  // Número de columnas basado en el tamaño del panel

            auto& viewports = ViewportManager::GetInstance().viewports;

            // Asegúrate de que hay al menos un viewport
            if (!viewports.empty()) {
                for (size_t i = 0; i < viewports.size(); ++i) {
                    Ref<Viewport> viewport = viewports[i];

                    // Comprobar si el viewport es válido
                    if (viewport != nullptr) {
                        // Desplegable para cada viewport
                        std::string viewportTitle = "Viewport " + std::to_string(i);

                        if (ImGui::TreeNode(viewportTitle.c_str())) {

                            // Botón para comprobar los FBO y GBO del viewport
                            if (ImGui::Button(("Check FBO-GBO (" + viewportTitle + ")").c_str())) {
                                viewport->CheckBuffers();  // Llamar a la función CheckBuffers del viewport correspondiente
                            }

                            if (ImGui::CollapsingHeader("GBuffer Textures")) {
                                if (viewport->gBuffer != nullptr) {
                                    glDisable(GL_DEPTH_TEST); // Deshabilitar test de profundidad

                                    ImGui::Columns(columns, nullptr, false);  // Establece las columnas

                                    // Dibuja las texturas del GBuffer
                                    DrawTexture("Position", viewport->gBuffer->getTexture("position"));
                                    DrawTexture("Depth", viewport->gBuffer->getTexture("depth"));
                                    DrawTexture("Position View", viewport->gBuffer->getTexture("positionView"));
                                    DrawTexture("Normal", viewport->gBuffer->getTexture("normal"));
                                    DrawTexture("Albedo", viewport->gBuffer->getTexture("albedo"));
                                    DrawTexture("Metallic Roughness", viewport->gBuffer->getTexture("metallicRoughness"));
                                    DrawTexture("Specular", viewport->gBuffer->getTexture("specular"));

                                    ImGui::Columns(1);  // Volver a una columna
                                    glEnable(GL_DEPTH_TEST); // Rehabilitar test de profundidad
                                }
                                else {
                                    ImGui::Text("No GBuffer available");
                                }
                            }

                            if (ImGui::CollapsingHeader("Framebuffers")) {
                                ImGui::Columns(columns, nullptr, false);  // Establece las columnas

                                // Dibuja las texturas de los Framebuffers
                                DrawTexture("Shadow Map Texture", viewport->framebuffer_shadowmap != nullptr ? viewport->framebuffer_shadowmap->getTexture("depth") : 0);
                                DrawTexture("SSAO Blur Texture",  viewport->framebuffer_SSAOBlur  != nullptr ? viewport->framebuffer_SSAOBlur->getTexture("color")  : 0);
                                DrawTexture("Deferred FBO",       viewport->framebuffer_deferred  != nullptr ? viewport->framebuffer_deferred->getTexture("color")  : 0);
                                DrawTexture("Forward FBO",        viewport->framebuffer_forward   != nullptr ? viewport->framebuffer_forward->getTexture("color")   : 0);
                                DrawTexture("Picking FBO",        viewport->framebuffer_picking   != nullptr ? viewport->framebuffer_picking->getTexture("color")   : 0);
                                DrawTexture("Final FBO",          viewport->framebuffer_final     != nullptr ? viewport->framebuffer_final->getTexture("color")     : 0);

                                ImGui::Columns(1);  // Volver a una columna
                            }

                            ImGui::TreePop();
                        }
                    }
                }
            }
            else {
                ImGui::Text("NO GAME VIEWPORT AVAILABLE");
            }

            ImGui::End();
        }






        





        //void Draw() override
        //{
        //    if (!m_isVisible) return;

        //    ImGui::Begin(m_title.c_str());

        //    // Slider para ajustar el tamaño de las imágenes
        //    ImGui::SliderFloat("Image Size", &m_ImageSize, 64.0f, 512.0f);

        //    // Obtener el tamaño del panel disponible
        //    ImVec2 panelSize = ImGui::GetContentRegionAvail();
        //    int columns = static_cast<int>(panelSize.x / m_ImageSize);

        //    if (ImGui::CollapsingHeader("GBuffer Textures")) {
        //        glDisable(GL_DEPTH_TEST); // Deshabilitar test de profundidad

        //        ImGui::Columns(columns, nullptr, false);

        //        DrawTexture("Position", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("position"));
        //        DrawTexture("Depth", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("depth"));
        //        DrawTexture("Position View", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("positionView"));
        //        DrawTexture("Normal", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("normal"));
        //        DrawTexture("Albedo", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("albedo"));
        //        DrawTexture("Metallic Roughness", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("metallicRoughness"));
        //        DrawTexture("Specular", ViewportManager::GetInstance().viewports[0]->gBuffer->getTexture("specular"));

        //        ImGui::Columns(1);  // Volver a una columna
        //        glEnable(GL_DEPTH_TEST); // Rehabilitar test de profundidad
        //    }

        //    if (ImGui::CollapsingHeader("Framebuffers")) {
        //        ImGui::Columns(columns, nullptr, false);

        //        DrawTexture("Shadow Map Texture", ViewportManager::GetInstance().viewports[0]->framebuffer_shadowmap->getTexture("depth"));
        //        DrawTexture("SSAO Blur Texture", ViewportManager::GetInstance().viewports[0]->framebuffer_SSAOBlur->getTexture("color"));
        //        DrawTexture("Deferred FBO", ViewportManager::GetInstance().viewports[0]->framebuffer_deferred->getTexture("color"));
        //        DrawTexture("Forward FBO", ViewportManager::GetInstance().viewports[0]->framebuffer_forward->getTexture("color"));
        //        DrawTexture("Picking FBO", ViewportManager::GetInstance().viewports[0]->framebuffer_picking->getTexture("color"));

        //        ImGui::Columns(1);  // Volver a una columna
        //    }

        //    ImGui::End();
        //}

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }

    private:
        float m_ImageSize;

        //void DrawTexture(const std::string& label, GLuint textureID) {
        //    if (textureID == 0) {
        //        std::cerr << "Error: " << label << " texture is invalid!" << std::endl;
        //        return;
        //    }
        //    ImGui::Text("%s", label.c_str());
        //    ImGui::Image((void*)(intptr_t)textureID, ImVec2(m_ImageSize, m_ImageSize), ImVec2(0, 1), ImVec2(1, 0));
        //    ImGui::NextColumn();  // Pasar a la siguiente columna
        //}

        void DrawTexture(const std::string& label, GLuint textureID) {
            if (textureID != 0) {
                ImGui::Text("%s", label.c_str());
                ImGui::Image((void*)(intptr_t)textureID, ImVec2(m_ImageSize, m_ImageSize), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::NextColumn();  // Avanzar a la siguiente columna
            }
            else {
                ImGui::Text("%s: Null", label.c_str());
                ImGui::NextColumn();  // Avanzar a la siguiente columna
            }
        }
    };
}
