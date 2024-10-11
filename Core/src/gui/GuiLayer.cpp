#include "GuiLayer.h"
#include "GuiStyle.h"

#include <imGizmo/ImGuizmo.h>
#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "../managers/WindowManager.h"

//--Panels
#include "Panels/InspectorPanel.h"
#include "Panels/FrameBuffersPreviewPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/EditorCameraPanel.h"
#include "Panels/AssetsPanel.h"
#include "Panels/ModelsPanel.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/LogPanel.h"
#include "Panels/MaterialsPanel.h"
#include "Panels/TexturesPanel.h"
#include "Panels/LuaPanel.h"
#include "Panels/GlobalIluminationPanel.h"
#include "Panels/DynamicSkyboxPanel.h"
#include "Panels/MeshesPanel.h"
#include "Panels/ScenePanel.h"


namespace libCore
{
    //--CONSTRUCTOR´s
    GuiLayer& GuiLayer::GetInstance()
    {
        static GuiLayer instance;  // Se crea solo una vez
        return instance;
    }
    GuiLayer::GuiLayer()
    {
        // Constructor privado
    }
    GuiLayer::~GuiLayer()
    {
        Shutdown();
    }
    //----------------------------------------------

    //--ImGUI CONFIGURATOR
    void GuiLayer::Init()
    {
        SetupImGui();

        //ViewportPanelEvents
        EventManager::OnPanelMouseEnterExitEvent().subscribe([this](const std::string& panelName, bool mouseEntered) {mouseInsideViewport = mouseEntered; });
        EventManager::OnPanelFocusEvent().subscribe([this](const std::string& panelName, bool isFocused) {mouseFocusedViewport = isFocused; });
        //--

        //ADD PANELS
        auto viewportPanel                 = CreateRef<ViewportPanel>();
        auto inspectorPanel                = CreateRef<InspectorPanel>();
        auto frameBuffersPreviewPanel      = CreateRef<FrameBuffersPreviewPanel>();
        auto editorCameraPanel             = CreateRef<EditorCameraPanel>();
        auto assetsPanel                   = CreateRef<AssetsPanel>();
        auto modelsPanel                   = CreateRef<ModelsPanel>();
        auto hierarchyPanel                = CreateRef<HierarchyPanel>();
        auto logPanel                      = CreateRef<LogPanel>();
        auto materialsPanel                = CreateRef<MaterialsPanel>();
        auto texturesPanel                 = CreateRef<TexturesPanel>();
        auto luaPanel                      = CreateRef<LuaPanel>();
        auto globalIluminationPanel        = CreateRef<GlobalIluminationPanel>();
        auto dynamicSkyboxPanel            = CreateRef<DynamicSkyboxPanel>();
        auto meshesPanel                   = CreateRef<MeshesPanel>();
        auto scenePanel                    = CreateRef<ScenePanel>();

       
        AddPanel(inspectorPanel);
        AddPanel(editorCameraPanel);
        AddPanel(assetsPanel);
        AddPanel(modelsPanel);
        //AddPanel(meshesPanel);
        AddPanel(scenePanel);
        AddPanel(hierarchyPanel);
        AddPanel(logPanel);
        AddPanel(materialsPanel);
        AddPanel(texturesPanel);
        AddPanel(luaPanel);
        AddPanel(globalIluminationPanel);
        AddPanel(frameBuffersPreviewPanel);
        AddPanel(dynamicSkyboxPanel);
        
        AddPanel(viewportPanel); //<- El ultimo
        //--

        // Inicializar todos los paneles
        for (auto& panel : m_panels)
        {
            panel->Init();
        }
    }
    void GuiLayer::DrawImGUI()
    {
        Draw();
    }
    void GuiLayer::Shutdown()
    {
        // Limpiar todos los paneles
        for (auto& panel : m_panels)
        {
            panel->Shutdown();
        }

        CleanupImGui();
    }
    void GuiLayer::SetupImGui()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Enable Docking
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;      // Enable Multi-Viewport / Platform Windows
        io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.ConfigDragClickToInputText = true;
        io.ConfigDockingTransparentPayload = true;


        //FONTS
        // Load custom fonts including FontAwesome
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;

        // Main font
        io.Fonts->AddFontFromFileTTF("assets/fonts/Dosis-Light.ttf", 16.0f);

        // FontAwesome
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromFileTTF("assets/fonts/Font_Awesome_5_Free-Solid-900.otf", 16.0f, &config, icons_ranges);

        io.Fonts->Build();
        //---



        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(WindowManager::GetInstance().GetWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 460");

        // Aplicar el estilo personalizado
        GuiStyle::ApplyCustomStyle();

        //ImGui::StyleColorsDark();
    }
    void GuiLayer::CleanupImGui()
    {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    //-------------------------------------------------------------------------------


    //--LIFE CYCLE
    void GuiLayer::begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }
    void GuiLayer::Draw()
    {
        // Begin a new frame for ImGui
        begin();

        // Main Menu Bar
        renderMainMenuBar();
        
        // Render the dock space
        renderDockers();

        //Render popUps
        PopupManager::GetInstance().ShowPopups(); // Llamada para renderizar todos los popups

        // Render the panels
        for (auto& panel : m_panels)
        {
            if (panel->IsVisible())
            {
                panel->Draw();
            }
        }

        //--CHECK ImGizmo
        //SE hace en el panel de ViewportPanel
        //--------------------------------------------------------

        //Render Tool bar
        DrawToolBarEditor();

        // Render ImGui elements and end frame
        end();
    }
    void GuiLayer::end()
    {

        ImGui::End(); // End of Docking Example window

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(WindowManager::GetInstance().GetWindowWidth(), WindowManager::GetInstance().GetWindowHeight());

        ImGui::Render();  // Solo es necesario este renderizado
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
    //-------------------------------------------------------------------------------


    //--CONTENEDORES PRINCIPALES -TOP MAIN MENU & TOOLBAR 
    void GuiLayer::renderDockers()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float menuBarHeight = ImGui::GetFrameHeight();  // Obtén la altura del menú principal
        float toolbarHeight = 40.0f;
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight + toolbarHeight));  // Coloca el docker debajo del menú
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight - toolbarHeight));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (ImGui::Begin("DockSpace", nullptr, window_flags))
        {
            ImGui::PopStyleVar(3);
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

            // Aquí se restaura el flag PassthruCentralNode
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

            //ImGui::End();  // Finaliza la ventana "DockSpace" correctamente
        }

        ImGui::PopStyleColor();

    }
    void GuiLayer::renderMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(ICON_FA_CUBE " GameObjects")) // Agregar el icono de un cubo
            {
                if (ImGui::MenuItem(ICON_FA_CUBE "Empty"))
                {
                    EntityManager::GetInstance().CreateEmptyGameObject("new_GameObject");
                }
                if (ImGui::BeginMenu(ICON_FA_LIGHTBULB " Lights"))
                {
                    if (ImGui::MenuItem(ICON_FA_SUN " Directional Light")) { /* Crear luz direccional */ }
                    if (ImGui::MenuItem(ICON_FA_LIGHTBULB " Point Light")) { /* Crear luz puntual */ }
                    if (ImGui::MenuItem(ICON_FA_RULER_COMBINED " Area Light")) { /* Crear luz de área */ }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(ICON_FA_CAMERA "Cameras"))
                {
                    if (ImGui::MenuItem(ICON_FA_CAMERA "Camera"))
                    {
                        EntityManager::GetInstance().CreateCamera();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(ICON_FA_USER " Entities"))
            {
                if (ImGui::MenuItem(ICON_FA_USERS " Check Entities")) { /* Acción para chequear entidades */ }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
    void GuiLayer::DrawToolBarEditor()
    {
        // Barra de herramientas ubicada debajo del menú superior
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, 40));

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        if (ImGui::Begin("Toolbar", nullptr, window_flags))
        {
            if (ImGui::Button(ICON_FA_STOP "")) {
                Engine::GetInstance().SetEngineState(EditorStates::PREPARE_STOP);
                //Engine::GetInstance().SetEngineState(EditorStates::STOP);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PAUSE "")) {
                Engine::GetInstance().SetEngineState(EditorStates::PAUSE);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PLAY "")) {
                Engine::GetInstance().SetEngineState(EditorStates::PREPARE_PLAY);
                //Engine::GetInstance().SetEngineState(EditorStates::PLAY);
            }

            //-- ImGIZMO CONTROLS AREA
            static bool useLocalTransform = true;
            static bool snapEnabled = false;
            static float snapValue[3] = { 1.0f, 1.0f, 1.0f };

            ImGui::SameLine();
            if (ImGui::Button(useLocalTransform ? ICON_FA_ARROWS_ALT " Switch to Global" : ICON_FA_COMPRESS " Switch to Local"))
            {
                useLocalTransform = !useLocalTransform;
            }
            ImGui::SameLine();
            if (ImGui::Button(snapEnabled ? ICON_FA_UNLINK " Disable Snap" : ICON_FA_LINK " Enable Snap"))
            {
                snapEnabled = !snapEnabled;
            }
        }
        ImGui::End();
    }
    //-------------------------------------------------------------------------------
    
    //--PANELS
    void GuiLayer::AddPanel(std::shared_ptr<PanelBase> panel)
    {
        m_panels.push_back(panel);
    }
    //-------------------------------------------------------------------------------

    //--ImGIZMO
    void GuiLayer::checkGizmo(Ref<Viewport> viewport)
    {
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

        //--INPUTS TOOLS
        if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_T))
        {
            GuiLayer::GetInstance().m_GizmoOperation = GizmoOperation::Translate;
        }
        else if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_Y))
        {
            GuiLayer::GetInstance().m_GizmoOperation = GizmoOperation::Rotate3D;
        }
        else if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_U))
        {
            GuiLayer::GetInstance().m_GizmoOperation = GizmoOperation::Scale;
        }

        //---------------------------ImGUIZMO------------------------------------------
        if (EntityManager::GetInstance().currentSelectedEntityInScene != entt::null)
        {
            ImGuizmo::SetOrthographic(false);

            // Obtener la posición y tamaño del viewport dentro de ImGui
            ImVec2 viewportPanelPos = ImGui::GetWindowPos();
            ImVec2 viewportPanelSize = ImGui::GetWindowSize();

            // Establecer el área donde se renderizará el gizmo en el panel de viewport
            ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, viewportPanelSize.x, viewportPanelSize.y);

            // Obtener las matrices de cámara (view) y proyección (projection)
            glm::mat4 camera_view = glm::lookAt(viewport->camera->Position, viewport->camera->Position + viewport->camera->Orientation, viewport->camera->Up);
            glm::mat4 camera_projection = viewport->camera->projection;

            // Obtener la transformación global de la entidad seleccionada
            auto& transformComponent = EntityManager::GetInstance().GetComponent<TransformComponent>(EntityManager::GetInstance().currentSelectedEntityInScene);
            glm::mat4 entity_transform = transformComponent.getGlobalTransform(EntityManager::GetInstance().currentSelectedEntityInScene, *EntityManager::GetInstance().m_registry);

            ImGuizmo::MODE transformMode = GuiLayer::GetInstance().m_useLocalTransform ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

            // Habilitar el snap si está activo
            bool snap = GuiLayer::GetInstance().m_snapEnabled;
            float* snapValues = snap ? GuiLayer::GetInstance().m_snapValue : nullptr;

            // Deshabilitar el Depth Test para evitar que se dibuje detrás de otros elementos
            //glDisable(GL_DEPTH_TEST);

            // Renderizar el gizmo
            ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection),
                GuiLayer::GetInstance().m_GizmoOperation == GizmoOperation::Translate ? ImGuizmo::TRANSLATE :
                GuiLayer::GetInstance().m_GizmoOperation == GizmoOperation::Rotate3D ? ImGuizmo::ROTATE :
                ImGuizmo::SCALE,
                transformMode, glm::value_ptr(entity_transform), nullptr, snapValues);

            // Habilitar nuevamente el Depth Test
            //glEnable(GL_DEPTH_TEST);


            if (ImGuizmo::IsOver())
            {
                Engine::GetInstance().usingGizmo = true;
            }
            else
            {
                Engine::GetInstance().usingGizmo = false;
            }

            if (ImGuizmo::IsUsing())
            {
                // Actualizar la transformación local basada en la transformación global manipulada
                transformComponent.setTransformFromGlobal(entity_transform, EntityManager::GetInstance().currentSelectedEntityInScene, *EntityManager::GetInstance().m_registry);
            }
        }
    }
    //-------------------------------------------------------------------------------
}
