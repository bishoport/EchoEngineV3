#include "PanelBase.h"
#include "../../core/renderer/Renderer.hpp"
#include "../../managers/ViewportManager.hpp"
#include "../../managers/EventManager.h"
#include "../GuiLayer.h"

namespace libCore
{
    class ViewportPanel : public PanelBase
    {
    public:
        ViewportPanel() : PanelBase("Viewport"), m_MouseOverPanel(false) {}

        void Init() override {
            // Inicialización del selector de texturas
            m_TextureOptions = { "Position", "Normal", "Albedo", "MetallicRoughness", "Specular", "Final" };
        }

        void Draw() override {
            if (!m_isVisible) return;
        
            // Empieza la ventana de ImGui con el título del panel
            ImGui::Begin(m_title.c_str());
        
            ImGui::Checkbox("Update Framebuffer Size", &m_updateFramebufferSize);

            // Obtén el Viewport desde el ViewportManager
            Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Asegúrate de que el índice es correcto
        
            if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
                // Verifica si el tamaño o posición del panel han cambiado
                ImVec2 panelSize = ImGui::GetContentRegionAvail();
                ImVec2 panelPos = ImGui::GetWindowPos();
        
                if (m_updateFramebufferSize)
                {
                    // Solo dispara el evento si el tamaño o posición cambian
                    if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
                        panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {

                        // Dispara el evento de cambio de tamaño y posición del panel
                        EventManager::OnPanelResizedEvent().trigger(m_title,
                            glm::vec2(panelSize.x, panelSize.y),
                            glm::vec2(panelPos.x, panelPos.y));
                    }
                }
                
        
                // Detectar si el mouse ha entrado o salido del panel
                bool isMouseOver = ImGui::IsWindowHovered();
                if (isMouseOver != m_MouseOverPanel) {
                    m_MouseOverPanel = isMouseOver;
                    EventManager::OnPanelMouseEnterExitEvent().trigger(m_title, m_MouseOverPanel);
                }
        
                // Detectar si el panel ha ganado o perdido el foco
                bool isFocused = ImGui::IsWindowFocused();
                if (isFocused != m_PanelFocused) {
                    m_PanelFocused = isFocused;
                    EventManager::OnPanelFocusEvent().trigger(m_title, m_PanelFocused);
                }
        
        
                // Dibuja el framebuffer en el panel de ImGui
                GLuint finalTexture = viewport->framebuffer_final->getTexture("color");
                ImGui::Image((void*)(intptr_t)finalTexture, panelSize, ImVec2(0, 1), ImVec2(1, 0));
        
                // Asegura la sincronización
                glFinish();
        
        
                GuiLayer::GetInstance().checkGizmo(ViewportManager::GetInstance().viewports[0]);
                //-------------------
            }
            ImGui::End(); // Asegúrate de cerrar la ventana de ImGui correctamente
        
                    
        }


        void Shutdown() override {
            // Aquí puedes limpiar los recursos si es necesario
        }

    private:
        std::vector<std::string> m_TextureOptions; // Lista de opciones del GBuffer y framebuffer final
        bool m_MouseOverPanel;                     // Variable para verificar si el mouse está sobre el panel
        bool m_PanelFocused;                       // Variable para verificar si el panel tiene el foco
        bool m_updateFramebufferSize;              // Variable para bloquear el Framebuffer rescaler y poder ver el Gbuffer
    };
}






//#pragma once
//#include "PanelBase.h"
//#include "../../core/renderer/Renderer.hpp"
//#include "../../managers/ViewportManager.hpp"
//#include "../../managers/EventManager.h"
//
//#include "../GuiLayer.h"
//
//namespace libCore
//{
//    class ViewportPanel : public PanelBase
//    {
//    public:
//        ViewportPanel() : PanelBase("Viewport"), m_MouseOverPanel(false) {}
//
//        void Init() override {
//            // Inicialización del checker
//            showInfoOverlay = false;  // Inicialmente el overlay está desactivado
//        }
//
//        void Draw() override {
//            if (!m_isVisible) return;
//
//            // Empieza la ventana de ImGui con el título del panel
//            ImGui::Begin(m_title.c_str());
//
//            // Obtén el Viewport desde el ViewportManager
//            Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Asegúrate de que el índice es correcto
//
//            if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
//                // Verifica si el tamaño o posición del panel han cambiado
//                ImVec2 panelSize = ImGui::GetContentRegionAvail();
//                ImVec2 panelPos = ImGui::GetWindowPos();
//
//                // Solo dispara el evento si el tamaño o posición cambian
//                if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
//                    panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {
//
//                    // Dispara el evento de cambio de tamaño y posición del panel
//                    EventManager::OnPanelResizedEvent().trigger(m_title,
//                        glm::vec2(panelSize.x, panelSize.y),
//                        glm::vec2(panelPos.x, panelPos.y));
//                }
//
//                // Detectar si el mouse ha entrado o salido del panel
//                bool isMouseOver = ImGui::IsWindowHovered();
//                if (isMouseOver != m_MouseOverPanel) {
//                    m_MouseOverPanel = isMouseOver;
//                    EventManager::OnPanelMouseEnterExitEvent().trigger(m_title, m_MouseOverPanel);
//                }
//
//                // Detectar si el panel ha ganado o perdido el foco
//                bool isFocused = ImGui::IsWindowFocused();
//                if (isFocused != m_PanelFocused) {
//                    m_PanelFocused = isFocused;
//                    EventManager::OnPanelFocusEvent().trigger(m_title, m_PanelFocused);
//                }
//
//
//                // Dibuja el framebuffer en el panel de ImGui
//                GLuint finalTexture = viewport->framebuffer_final->getTexture("color");
//                ImGui::Image((void*)(intptr_t)finalTexture, panelSize, ImVec2(0, 1), ImVec2(1, 0));
//
//                // Asegura la sincronización
//                glFinish();
//
//
//                GuiLayer::GetInstance().checkGizmo(ViewportManager::GetInstance().viewports[0]);
//                //-------------------
//            }
//            ImGui::End(); // Asegúrate de cerrar la ventana de ImGui correctamente
//
//            
//        }
//
//        void Shutdown() override {
//            // Aquí puedes limpiar los recursos si es necesario
//        }
//
//    private:
//        bool showInfoOverlay;  // Variable para controlar si se muestra el label con la información
//        bool m_MouseOverPanel; // Variable para verificar si el mouse está sobre el panel
//        bool m_PanelFocused; // Variable para verificar si el mouse está sobre el panel
//    };
//}