#include "PanelBase.h"
#include "../../core/Engine.h"
#include "../../core/renderer/Renderer.hpp"
#include "../../managers/ViewportManager.hpp"
#include "../../managers/EventManager.h"
#include "../GuiLayer.h"

namespace libCore
{
    class GameViewportPanel : public PanelBase
    {
    public:
        GameViewportPanel()
            : PanelBase("Game"),
            m_MouseOverPanel(false),
            m_PanelFocused(false),
            m_updateFramebufferSize(false) {}  // Inicializamos m_updateFramebufferSize a false

        void Init() override {
            // Inicializaci�n del selector de texturas
            m_TextureOptions = { "Position", "Normal", "Albedo", "MetallicRoughness", "Specular", "Final" };
        }

        void Draw() override {

            if (!m_isVisible) return;

            // Empieza la ventana de ImGui con el t�tulo del panel
            ImGui::Begin(m_title.c_str());

            if (ViewportManager::GetInstance().viewports.size() > 1)
            {
                // Obt�n el Viewport desde el ViewportManager
                Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[1];  // Aseg�rate de que el �ndice es correcto

                if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
                    // Verifica si el tama�o o posici�n del panel han cambiado
                    ImVec2 panelSize = ImGui::GetContentRegionAvail();
                    ImVec2 panelPos = ImGui::GetWindowPos();

                    // Compara el tama�o y la posici�n del viewport con los del panel
                    if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
                        panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {


                        // Actualiza la posici�n y tama�o del Viewport
                        viewport->viewportSize = glm::vec2(panelSize.x, panelSize.y);
                        viewport->viewportPos = glm::vec2(panelPos.x, panelPos.y);  // Esta posici�n ahora ser� la posici�n global corregida desde el panel

                        // Redimensionar los framebuffers
                        viewport->ResizeBuffers(static_cast<int>(panelSize.x), static_cast<int>(panelSize.y));

                        // Actualizar el tama�o de la c�mara
                        viewport->UpdateCameraSizeView(static_cast<int>(panelSize.x), static_cast<int>(panelSize.y));

                        //// Dispara el evento de cambio de tama�o y posici�n del panel
                        //EventManager::OnPanelResizedEvent().trigger(m_title,
                        //    glm::vec2(panelSize.x, panelSize.y),
                        //    glm::vec2(panelPos.x, panelPos.y)); // Posici�n y tama�o globales corregidos

                        // Actualiza las dimensiones del viewport
                        //viewport->viewportSize = glm::vec2(panelSize.x, panelSize.y);
                        //viewport->viewportPos = glm::vec2(panelPos.x, panelPos.y);
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
                    if (finalTexture != 0) {
                        ImGui::Image((void*)(intptr_t)finalTexture, panelSize, ImVec2(0, 1), ImVec2(1, 0));
                    }
                    else {
                        ImGui::Text("No texture available for this framebuffer");
                    }

                    // Asegura la sincronizaci�n
                    glFinish();
                }
            }
            else
            {
                ImGui::Text("NO GAME VIEWPORT AVAILABLE");
            }

            ImGui::End();
        }

        void Shutdown() override {
            // Aqu� puedes limpiar los recursos si es necesario
        }

    private:
        std::vector<std::string> m_TextureOptions;  // Lista de opciones del GBuffer y framebuffer final
        bool m_MouseOverPanel;                      // Variable para verificar si el mouse est� sobre el panel
        bool m_PanelFocused;                        // Variable para verificar si el panel tiene el foco
        bool m_updateFramebufferSize;               // Variable para bloquear el Framebuffer rescaler y poder ver el GBuffer
    };
}


//#include "PanelBase.h"
//#include "../../core/Engine.h"
//#include "../../core/renderer/Renderer.hpp"
//#include "../../managers/ViewportManager.hpp"
//#include "../../managers/EventManager.h"
//#include "../GuiLayer.h"
//
//namespace libCore
//{
//    class GameViewportPanel : public PanelBase
//    {
//    public:
//        GameViewportPanel() : PanelBase("Game"), m_MouseOverPanel(false) {}
//
//        void Init() override {
//            // Inicializaci�n del selector de texturas
//            m_TextureOptions = { "Position", "Normal", "Albedo", "MetallicRoughness", "Specular", "Final" };
//        }
//
//
//
//        void Draw() override {
//
//            if (!m_isVisible) return;
//
//            // Empieza la ventana de ImGui con el t�tulo del panel
//            ImGui::Begin(m_title.c_str());
//
//            if (ViewportManager::GetInstance().viewports.size() > 1)
//            {
//                // Obt�n el Viewport desde el ViewportManager
//                Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[1];  // Aseg�rate de que el �ndice es correcto
//
//                if (viewport != nullptr && viewport->framebuffer_final != nullptr) {
//                    // Verifica si el tama�o o posici�n del panel han cambiado
//                    ImVec2 panelSize = ImGui::GetContentRegionAvail();
//                    ImVec2 panelPos = ImGui::GetWindowPos();
//
//                    if (m_updateFramebufferSize) {
//                        // Solo dispara el evento si el tama�o o posici�n cambian
//                        if (panelSize.x != viewport->viewportSize.x || panelSize.y != viewport->viewportSize.y ||
//                            panelPos.x != viewport->viewportPos.x || panelPos.y != viewport->viewportPos.y) {
//
//                            // Dispara el evento de cambio de tama�o y posici�n del panel
//                            EventManager::OnPanelResizedEvent().trigger(m_title,
//                                glm::vec2(panelSize.x, panelSize.y),
//                                glm::vec2(panelPos.x, panelPos.y)); // Posici�n y tama�o globales corregidos
//                        }
//                    }
//
//                    // Detectar si el mouse ha entrado o salido del panel
//                    bool isMouseOver = ImGui::IsWindowHovered();
//                    if (isMouseOver != m_MouseOverPanel) {
//                        m_MouseOverPanel = isMouseOver;
//                        EventManager::OnPanelMouseEnterExitEvent().trigger(m_title, m_MouseOverPanel);
//                    }
//
//                    // Detectar si el panel ha ganado o perdido el foco
//                    bool isFocused = ImGui::IsWindowFocused();
//                    if (isFocused != m_PanelFocused) {
//                        m_PanelFocused = isFocused;
//                        EventManager::OnPanelFocusEvent().trigger(m_title, m_PanelFocused);
//                    }
//
//                    // Dibuja el framebuffer en el panel de ImGui
//                    GLuint finalTexture = viewport->framebuffer_final->getTexture("color");
//                    ImGui::Image((void*)(intptr_t)finalTexture, panelSize, ImVec2(0, 1), ImVec2(1, 0));
//
//                    // Asegura la sincronizaci�n
//                    glFinish();
//                }
//            }
//            else
//            {
//                ImGui::Text("NO GAME VIEWPORT AVAILABLE");
//            }
//
//            ImGui::End();
//        }
//
//        void Shutdown() override {
//            // Aqu� puedes limpiar los recursos si es necesario
//        }
//
//    private:
//        std::vector<std::string> m_TextureOptions; // Lista de opciones del GBuffer y framebuffer final
//        bool m_MouseOverPanel;                     // Variable para verificar si el mouse est� sobre el panel
//        bool m_PanelFocused;                       // Variable para verificar si el panel tiene el foco
//        bool m_updateFramebufferSize;              // Variable para bloquear el Framebuffer rescaler y poder ver el Gbuffer
//    };
//}






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