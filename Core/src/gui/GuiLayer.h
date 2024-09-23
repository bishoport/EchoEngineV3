#pragma once
#include "../core/Core.h"
#include "../core/renderer/Viewport.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Panels/PanelBase.h"

#include "PopupManager.h"

namespace libCore
{
    class GuiLayer
    {
    public:
        
        //--Instance
        static GuiLayer& GetInstance();
        GuiLayer(const GuiLayer&) = delete;
        GuiLayer& operator=(const GuiLayer&) = delete;
        //--

        //-- PUBLIC LIFE CYCLE
        void Init();
        void DrawImGUI();
        void Shutdown();
        //--
        
        //--ImGuizmo
        GizmoOperation m_GizmoOperation;
        bool isSelectingObject = false;
        bool showModelSelectionCombo = false;
        bool mouseInsideViewport = false;
        bool mouseFocusedViewport = false;
        void checkGizmo(Ref<Viewport> viewport);
        bool m_useLocalTransform = true; // Variable para controlar el modo de transformación
        bool m_snapEnabled = false; // Variable para controlar si el snap está habilitado
        float m_snapValue[3] = { 1.0f, 1.0f, 1.0f }; // Valores de snap para mover, rotar, y escalar
        //--

    private:
        // Constructor y destructor privados para garantizar que solo se pueda crear desde dentro de la clase
        GuiLayer();
        ~GuiLayer();

        //--CONFIG
        void SetupImGui();
        void CleanupImGui();

        //--PRIVATE LIFE CYCLE
        void begin();
        void Draw();
        void end();
        //--------------------

        //--CONTENEDORES PRINCIPALES
        void DrawToolBarEditor();
        void renderDockers();
        void renderMainMenuBar();
        //-----------------------------------------------

        //--PANELS
        void AddPanel(Ref<PanelBase> panel);
        std::vector<Ref<PanelBase>> m_panels;
        //-----------------------------------------------
    };
}
