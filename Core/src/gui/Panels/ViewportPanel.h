#include "PanelBase.h"
#include "../../core/Engine.h"
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
            // Inicializaci�n del selector de texturas
            m_TextureOptions = { "Position", "Normal", "Albedo", "MetallicRoughness", "Specular", "Final" };
        }

        void Draw() override {
            if (!m_isVisible) return;

            // Empieza la ventana de ImGui con el t�tulo del panel
            ImGui::Begin(m_title.c_str());

            // Obt�n el Viewport desde el ViewportManager
            Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Aseg�rate de que el �ndice es correcto

            if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
                // Verifica si el tama�o o posici�n del panel han cambiado
                ImVec2 panelSize = ImGui::GetContentRegionAvail();
                ImVec2 panelPos = ImGui::GetWindowPos();

                if (m_updateFramebufferSize) {
                    // Solo dispara el evento si el tama�o o posici�n cambian
                    if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
                        panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {

                        // Dispara el evento de cambio de tama�o y posici�n del panel
                        EventManager::OnPanelResizedEvent().trigger(m_title,
                            glm::vec2(panelSize.x, panelSize.y),
                            glm::vec2(panelPos.x, panelPos.y)); // Posici�n y tama�o globales corregidos
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

                // Asegura la sincronizaci�n
                glFinish();

                GuiLayer::GetInstance().checkGizmo(ViewportManager::GetInstance().viewports[0]);
            }
            ImGui::End();
        }


        //void Draw() override {
        //    if (!m_isVisible) return;

        //    // Empieza la ventana de ImGui con el t�tulo del panel
        //    ImGui::Begin(m_title.c_str());

        //    //ImGui::Checkbox("Update Framebuffer Size", &m_updateFramebufferSize);

        //    // Obt�n el Viewport desde el ViewportManager
        //    Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Aseg�rate de que el �ndice es correcto

        //      ImGui::Text("Panel Pos X: %.1f, Panel Pos Y: %.1f", viewport->viewportPos.x, viewport->viewportPos.y);
        //      ImGui::SliderInt("offset X", &Engine::GetInstance().offSetMouseX, -200, 200);
        //      ImGui::SliderInt("offset Y", &Engine::GetInstance().offSetMouseY, -200, 200);

        //    

        //    if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
        //        // Verifica si el tama�o o posici�n del panel han cambiado
        //        ImVec2 panelSize = ImGui::GetContentRegionAvail();
        //        ImVec2 panelPos = ImGui::GetWindowPos();

        //        m_updateFramebufferSize = true;
        //        if (m_updateFramebufferSize)
        //        {
        //            // Solo dispara el evento si el tama�o o posici�n cambian
        //            if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
        //                panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {

        //                // Dispara el evento de cambio de tama�o y posici�n del panel
        //                EventManager::OnPanelResizedEvent().trigger(m_title,
        //                    glm::vec2(panelSize.x, panelSize.y),
        //                    glm::vec2(panelPos.x, panelPos.y));
        //            }
        //        }


        //        // Detectar si el mouse ha entrado o salido del panel
        //        bool isMouseOver = ImGui::IsWindowHovered();
        //        if (isMouseOver != m_MouseOverPanel) {
        //            m_MouseOverPanel = isMouseOver;
        //            EventManager::OnPanelMouseEnterExitEvent().trigger(m_title, m_MouseOverPanel);
        //        }

        //        // Detectar si el panel ha ganado o perdido el foco
        //        bool isFocused = ImGui::IsWindowFocused();
        //        if (isFocused != m_PanelFocused) {
        //            m_PanelFocused = isFocused;
        //            EventManager::OnPanelFocusEvent().trigger(m_title, m_PanelFocused);
        //        }


        //        // Dibuja el framebuffer en el panel de ImGui
        //        GLuint finalTexture = viewport->framebuffer_final->getTexture("color");
        //        ImGui::Image((void*)(intptr_t)finalTexture, panelSize, ImVec2(0, 1), ImVec2(1, 0));

        //        // Asegura la sincronizaci�n
        //        glFinish();


        //        GuiLayer::GetInstance().checkGizmo(ViewportManager::GetInstance().viewports[0]);
        //        //-------------------
        //    }
        //    ImGui::End(); // Aseg�rate de cerrar la ventana de ImGui correctamente
        //}

        //void Draw() override {
        //    if (!m_isVisible) return;
        //
        //    // Obt�n el Viewport desde el ViewportManager
        //    Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Aseg�rate de que el �ndice es correcto

        //    // Empieza la ventana de ImGui con el t�tulo del panel
        //    ImGui::Begin(m_title.c_str());
        //
        //    //ImGui::Checkbox("Update Framebuffer Size", &m_updateFramebufferSize);
        //    //ImGui::SameLine();
        //    //float mouseX, mouseY;
        //    //std::tie(mouseX, mouseY) = InputManager::Instance().GetMousePosition();
        //    //ImGui::Text("Mouse X: %.1f, Y: %.1f", mouseX, mouseY);

        //    ImGui::Text("Panel Pos X: %.1f, Panel Pos Y: %.1f", viewport->viewportPos.x, viewport->viewportPos.y);
        //    ImGui::SliderInt("offset X", &Engine::GetInstance().offSetMouseX, -200, 200);
        //    ImGui::SliderInt("offset Y", &Engine::GetInstance().offSetMouseY, -200, 200);

        //    
        //
        //    if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
        //        // Verifica si el tama�o o posici�n del panel han cambiado
        //        ImVec2 panelSize = ImGui::GetContentRegionAvail();
        //        ImVec2 panelPos = ImGui::GetWindowPos();
        //
        //        if (m_updateFramebufferSize)
        //        {
        //            // Solo dispara el evento si el tama�o o posici�n cambian
        //            if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
        //                panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {

        //                // Dispara el evento de cambio de tama�o y posici�n del panel
        //                EventManager::OnPanelResizedEvent().trigger(m_title,
        //                    glm::vec2(panelSize.x, panelSize.y),
        //                    glm::vec2(panelPos.x, panelPos.y));
        //            }
        //        }
        //        
        //
        //        // Detectar si el mouse ha entrado o salido del panel
        //        bool isMouseOver = ImGui::IsWindowHovered();
        //        if (isMouseOver != m_MouseOverPanel) {
        //            m_MouseOverPanel = isMouseOver;
        //            EventManager::OnPanelMouseEnterExitEvent().trigger(m_title, m_MouseOverPanel);
        //        }
        //
        //        // Detectar si el panel ha ganado o perdido el foco
        //        bool isFocused = ImGui::IsWindowFocused();
        //        if (isFocused != m_PanelFocused) {
        //            m_PanelFocused = isFocused;
        //            EventManager::OnPanelFocusEvent().trigger(m_title, m_PanelFocused);
        //        }

        //        // Dibuja el framebuffer en el panel de ImGui
        //        GLuint finalTexture = viewport->framebuffer_final->getTexture("color");
        //        ImGui::Image((void*)(intptr_t)finalTexture, panelSize, ImVec2(0, 1), ImVec2(1, 0));
        //
        //        // Asegura la sincronizaci�n
        //        glFinish();
        //
        //
        //        GuiLayer::GetInstance().checkGizmo(ViewportManager::GetInstance().viewports[0]);
        //        //-------------------
        //    }
        //    ImGui::End(); // Aseg�rate de cerrar la ventana de ImGui correctamente
        //
        //            
        //}

        




        void Shutdown() override {
            // Aqu� puedes limpiar los recursos si es necesario
        }

    private:
        std::vector<std::string> m_TextureOptions; // Lista de opciones del GBuffer y framebuffer final
        bool m_MouseOverPanel;                     // Variable para verificar si el mouse est� sobre el panel
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
//            // Inicializaci�n del checker
//            showInfoOverlay = false;  // Inicialmente el overlay est� desactivado
//        }
//
//        void Draw() override {
//            if (!m_isVisible) return;
//
//            // Empieza la ventana de ImGui con el t�tulo del panel
//            ImGui::Begin(m_title.c_str());
//
//            // Obt�n el Viewport desde el ViewportManager
//            Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Aseg�rate de que el �ndice es correcto
//
//            if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
//                // Verifica si el tama�o o posici�n del panel han cambiado
//                ImVec2 panelSize = ImGui::GetContentRegionAvail();
//                ImVec2 panelPos = ImGui::GetWindowPos();
//
//                // Solo dispara el evento si el tama�o o posici�n cambian
//                if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
//                    panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {
//
//                    // Dispara el evento de cambio de tama�o y posici�n del panel
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
//                // Asegura la sincronizaci�n
//                glFinish();
//
//
//                GuiLayer::GetInstance().checkGizmo(ViewportManager::GetInstance().viewports[0]);
//                //-------------------
//            }
//            ImGui::End(); // Aseg�rate de cerrar la ventana de ImGui correctamente
//
//            
//        }
//
//        void Shutdown() override {
//            // Aqu� puedes limpiar los recursos si es necesario
//        }
//
//    private:
//        bool showInfoOverlay;  // Variable para controlar si se muestra el label con la informaci�n
//        bool m_MouseOverPanel; // Variable para verificar si el mouse est� sobre el panel
//        bool m_PanelFocused; // Variable para verificar si el mouse est� sobre el panel
//    };
//}