#include "Engine.h"

#include "../managers/WindowManager.h"
#include "../managers/ShaderManager.h"
#include "../managers/AssetsManager.h"
#include "../managers/ViewportManager.hpp"
#include "../managers/BackgroundTaskManager.h"

#include "../tools/ConsoleLog.h"
#include "../gui/GuiLayer.h"

#include "../managers/SceneManager.h"

namespace libCore
{
    //--INIT & LIFE CYCLE
    bool Engine::InitializeEngine(const std::string& windowTitle, int initialWindowWidth, int initialWindowHeight, bool full_play)
    {
        if (full_play == true)
        {
			m_EngineMode = FULL_PLAY_MODE;
        }

        // Utiliza WindowManager para inicializar la ventana
        WindowManager& windowManager = WindowManager::GetInstance();

        // Inicializa la ventana con el título y dimensiones especificadas
        if (!windowManager.Initialize(initialWindowWidth, initialWindowHeight, windowTitle.c_str())) {
            return false;
        }

        // Obtener el puntero a la ventana GLFW desde el WindowManager
        GLFWwindow* window = windowManager.GetWindow();
        if (!window) {
            std::cerr << "Failed to initialize window." << std::endl;
            return false;
        }

        //--Glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Failed to initialize GLAD");
            return false;
        }
        //-------------------------------------------------


		//--GL debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(ConsoleLog::MessageCallback, 0);

		// Filter out notification messages
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

		// Optionally, you can further filter by source and type
		// Example: Ignore performance warnings
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);

		// Example: Ignore all messages except for high severity errors
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
		//-----------------------------


        // -- OPENGL FLAGS
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glEnable(GL_MULTISAMPLE);  // always enable to ensure multisampling

		// -- SHADERS
		std::string shadersDirectory = "assets/shaders/";
		ShaderManager::setShaderDataLoad("colorQuadFBO", shadersDirectory + "quad_fbo.vert", shadersDirectory + "color_quad_fbo.frag");

		//-DEBUG & TEXT
		ShaderManager::setShaderDataLoad("debug",    shadersDirectory + "debug.vert",    shadersDirectory + "debug.frag");
		ShaderManager::setShaderDataLoad("snapshot", shadersDirectory + "snapshot.vert", shadersDirectory + "snapshot.frag");
		ShaderManager::setShaderDataLoad("text",     shadersDirectory + "text.vert",     shadersDirectory + "text.frag");
		ShaderManager::setShaderDataLoad("grid",     shadersDirectory + "grid.vert",     shadersDirectory + "grid.frag");
		ShaderManager::setShaderDataLoad("axes",     shadersDirectory + "axes.vert",     shadersDirectory + "axes.frag");
		ShaderManager::setShaderDataLoad("frustum",  shadersDirectory + "frustum.vert",  shadersDirectory + "frustum.frag");

		//-SSAO
		ShaderManager::setShaderDataLoad("ssao",     shadersDirectory + "ssao.vert",     shadersDirectory + "ssao.frag");
		ShaderManager::setShaderDataLoad("ssaoBlur", shadersDirectory + "ssao.vert",     shadersDirectory + "ssao_blur.frag");

		//-SKYBOX
		ShaderManager::setShaderDataLoad("dynamicSkybox", shadersDirectory + "skybox/dynamicSkybox.vs", shadersDirectory + "skybox/dynamicSkybox.fs");

		//-G_Buffer
		ShaderManager::setShaderDataLoad("geometryPass", shadersDirectory + "deferred/geometryPass.vert", shadersDirectory + "deferred/geometryPass.frag");
		ShaderManager::setShaderDataLoad("lightingPass", shadersDirectory + "deferred/lightingPass.vert", shadersDirectory + "deferred/lightingPass.frag");
		ShaderManager::setShaderDataLoad("combinePass", shadersDirectory + "combine.vert", shadersDirectory + "combine.frag");

		//-IBL
		ShaderManager::setShaderDataLoad("equirectangularToCubemap", shadersDirectory + "IBL/cubemap.vs", shadersDirectory + "IBL/equirectangular_to_cubemap.fs");
		ShaderManager::setShaderDataLoad("irradiance", shadersDirectory + "IBL/cubemap.vs", shadersDirectory + "IBL/irradiance_convolution.fs");
		ShaderManager::setShaderDataLoad("prefilter", shadersDirectory + "IBL/cubemap.vs", shadersDirectory + "IBL/prefilter.fs");
		ShaderManager::setShaderDataLoad("brdf", shadersDirectory + "IBL/brdf.vs", shadersDirectory + "IBL/brdf.fs");
		ShaderManager::setShaderDataLoad("background", shadersDirectory + "IBL/background.vs", shadersDirectory + "IBL/background.fs");
		ShaderManager::setShaderDataLoad("captureIBL", shadersDirectory + "IBL/capture_enviroment.vert", shadersDirectory + "IBL/capture_enviroment.frag");

		//-SHADOWS
		ShaderManager::setShaderDataLoad("direct_light_depth_shadows", shadersDirectory + "shadows/directLight_shadow_mapping_depth_shader.vert", shadersDirectory + "shadows/directLight_shadow_mapping_depth_shader.fs");

		//-STENCIL MOUSE PICKING
		ShaderManager::setShaderDataLoad("picking", shadersDirectory + "picking.vert", shadersDirectory + "picking.frag");
		ShaderManager::LoadAllShaders();
		//-----------------------------------------------------------------


		// -- ASSETS MANAGER (load Default Assets)
		AssetsManager::GetInstance().LoadDefaultAssets();
		AssetsManager::GetInstance().CreateDefaultMaterial();
		//---------------------------------------------------------------------------

		ImportModelData importModelData;
		importModelData.filePath = "assets/models/";
		importModelData.fileName = "Primitives.glb";
		importModelData.invertUV = false;
		importModelData.rotate90 = false;
		importModelData.skeletal = false;
		importModelData.modelID = 0;
		importModelData.globalScaleFactor = 1.0f;
		AssetsManager::GetInstance().LoadModelAsset(importModelData);

		//PREPARE ENGINE MODE
        if (m_EngineMode == EDITOR)
        {
            // -- IMGUI
            GuiLayer::GetInstance().Init();
            // -- VIEWPORT
            ViewportManager::GetInstance().CreateViewport("EDITOR CAMERA", glm::vec3(0.0f, 20.0f, 0.0f), 800, 600, CAMERA_CONTROLLERS::EDITOR);
        }
        else if (m_EngineMode == FULL_PLAY_MODE)
        {
            // -- IMGUI
            GuiLayer::GetInstance().Init();
            // -- VIEWPORT
            ViewportManager::GetInstance().CreateViewport("FULL_PLAY CAMERA", glm::vec3(0.0f, 20.0f, 0.0f), 800, 600, CAMERA_CONTROLLERS::GAME);
        }
		// -------------------------------------------------


        // -- RENDERER
        Renderer::getInstance().initialize();
        //---------------------------------------------------------------------------

		//--INITIAL-STATE
		if (m_EngineMode == EDITOR)
		{
			m_CurrentState = STOP;
		}
		else
		{
			m_CurrentState = STOP;
		}
		//---------------------------------------------------------------------------

		//--INITIAL-SCENE
		SceneManager::GetInstance().CreateNewScene("new_scene");
		SceneManager::GetInstance().SetupSceneManager();
		//---------------------------------------------------------------------------

		return true;
    }
    void Engine::InitializeMainLoop()
    {
        Timestep lastFrameTime = static_cast<float>(glfwGetTime());

        running = true;

		BackgroundTaskManager::GetInstance().Start();

        while (!WindowManager::GetInstance().ShouldClose() && running)
        {

			MainThreadTaskManager::GetInstance().ExecuteTasks();

            //--DELTA TIME
            Timestep currentFrameTime = static_cast<float>(glfwGetTime());
            m_deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
			//---------

			
			if (m_CurrentState != BUSY)
			{
				//--UPDATE SCENE
				UpdateBeforeRender();
				//---------

				//--RENDER SCENE
				Renderer::getInstance().RenderViewport(ViewportManager::GetInstance().viewports[0], m_deltaTime);
				//---------

				EntityManager::GetInstance().DestroyDeleteMarked();


				//--VIEWPORT OUT
				if (m_EngineMode == EDITOR)
				{
					glDisable(GL_DEPTH_TEST);  // Desactiva el depth test para que el gizmo se dibuje en la parte superior
					glEnable(GL_BLEND);
					//--RENDER IN ViewportPanel
					GuiLayer::GetInstance().DrawImGUI();
					// Restaurar el estado OpenGL tras renderizar ImGui/ImGuizmo
					glEnable(GL_DEPTH_TEST);
				}
				else if (m_EngineMode == FULL_PLAY_MODE)
				{
					//--RENDER IN Quad
					Renderer::getInstance().ShowViewportInQuad(ViewportManager::GetInstance().viewports[0]);
				}
				//---------
			}
			

            

            //--CLOSE INPUT
            InputManager::Instance().EndFrame();

            //--CLOSE FRAME
            WindowManager::GetInstance().PollEvents();
            WindowManager::GetInstance().SwapBuffers();
        }

		BackgroundTaskManager::GetInstance().Stop();
        glfwTerminate();
    }
    void Engine::StopMainLoop()
    {
        running = false;
    }
	// -------------------------------------------------


	//--UPDATES
	void Engine::UpdateBeforeRender()
	{
		//--INPUT UPDATE
		InputManager::Instance().Update();

		//--CONTROL_KEY + N
		if (InputManager::Instance().IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || InputManager::Instance().IsKeyPressed(GLFW_KEY_RIGHT_CONTROL))
		{
			if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_D))
			{
				EntityManager::GetInstance().DuplicateEntity();
			}
			else if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_W))
			{
				Renderer::getInstance().m_wireframe = !Renderer::getInstance().m_wireframe;
			}
			else if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_M))
			{
				Renderer::getInstance().enableMultisample = !Renderer::getInstance().enableMultisample;
			}
		}
		else
		{
			//--ImGizmo
			if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_1))
			{
				GuiLayer::GetInstance().m_GizmoOperation = GizmoOperation::Translate;
			}
			else if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_2))
			{
				GuiLayer::GetInstance().m_GizmoOperation = GizmoOperation::Rotate3D;
			}
			else if (InputManager::Instance().IsKeyJustPressed(GLFW_KEY_3))
			{
				GuiLayer::GetInstance().m_GizmoOperation = GizmoOperation::Scale;
			}
		}
		//------------------------------------------------------------------------------
		//------------------------------------------------------------------------------

		//--UPDATE DynamicSkyBox & SunLight
		//Renderer::getInstance().dynamicSkybox->sunLight->UpdateSceneRadius();
		//Renderer::getInstance().dynamicSkybox->SyncSunLightWithSkybox();
		//-------------------------------------------------------------------

		//--UPDATE ENTITIES WITH TRANSFORM & AABB
		EntityManager::GetInstance().UpdateGameObjects(m_deltaTime);
		//-------------------------------------------

		//--UPDATE GAME CAMERA IN VIEWPORT
		/*entt::entity mainCameraEntity = EntityManager::GetInstance().GetEntityByName("MainCamera");
		if (mainCameraEntity != entt::null)
		{
			auto& cameraComponent = EntityManager::GetInstance().GetComponent<CameraComponent>(mainCameraEntity);
			auto& transformComponent = EntityManager::GetInstance().GetComponent<TransformComponent>(mainCameraEntity);

			if (cameraComponent.camera == nullptr)
			{
				cameraComponent.camera = ViewportManager::GetInstance().viewports[0]->camera;
			}

			ViewportManager::GetInstance().viewports[0]->camera->Position = transformComponent.transform->GetPosition();
		}*/


		/*if (engineState == EDITOR || engineState == EDITOR_PLAY)
		{
			ViewportManager::GetInstance().viewports[0]->camera = ViewportManager::GetInstance().viewports[0]->camera;
		}
		else if (engineState == PLAY)
		{
			ViewportManager::GetInstance().viewports[0]->camera = ViewportManager::GetInstance().viewports[0]->gameCamera;
		}*/

		if (m_EngineMode == EngineMode::EDITOR_MODE && GuiLayer::GetInstance().mouseInsideViewport == true)
		{
			//--MOUSE PICKING (Color Picking)
			if (!usingGizmo)
			{
				if (GuiLayer::GetInstance().isSelectingObject == true)
				{
					return;  // Salir si ya estamos en proceso de selección
				}

				GuiLayer::GetInstance().isSelectingObject = false;

				// Obtener la posición del mouse
				float mouseX, mouseY;
				std::tie(mouseX, mouseY) = InputManager::Instance().GetMousePosition();

				// Verificar si se ha presionado el botón izquierdo del mouse
				if (InputManager::Instance().IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT) && !GuiLayer::GetInstance().isSelectingObject)
				{
					// Solo ejecutar la lógica de picking cuando se presiona el botón
					Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Asegúrate de que el índice es correcto

					// Aplicar el offset ajustado por el usuario

					Engine::GetInstance().offSetMouseY = -viewport->viewportSize.y  - 25;
					Engine::GetInstance().offSetMouseX = -10;

					float relativeMouseX = mouseX - viewport->viewportPos.x + Engine::GetInstance().offSetMouseX;
					float relativeMouseY = mouseY - (viewport->viewportPos.y - viewport->viewportSize.y) + Engine::GetInstance().offSetMouseY;

					// Asegurarse de que el click está dentro de los límites del Viewport
					if (relativeMouseX >= 0 && relativeMouseX <= viewport->viewportSize.x &&
						relativeMouseY >= 0 && relativeMouseY <= viewport->viewportSize.y)
					{
						// Ajustar las coordenadas Y si es necesario para OpenGL (inversión del eje Y dentro del viewport)
						int adjustedMouseY = viewport->viewportSize.y - static_cast<int>(relativeMouseY);

						// Leer el color del framebuffer de picking
						unsigned char pixelColor[3];
						glBindFramebuffer(GL_READ_FRAMEBUFFER, viewport->framebuffer_picking->getFramebuffer());
						glReadPixels(static_cast<int>(relativeMouseX), adjustedMouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixelColor);
						glBindFramebuffer(GL_FRAMEBUFFER, 0); // Desvincular el framebuffer

						// Convertir el color a un UUID (directamente 24 bits)
						uint32_t pickedID = EntityManager::GetInstance().ColorToUUID(pixelColor[0], pixelColor[1], pixelColor[2]);

						// Verificar si se ha seleccionado un objeto
						if (pickedID > 0)  // Asumiendo que un ID válido es mayor que 0
						{
							std::cout << "Objeto seleccionado con UUID: " << pickedID << std::endl;
							EntityManager::GetInstance().currentSelectedEntityInScene = EntityManager::GetInstance().GetEntityByUUID(pickedID);
							GuiLayer::GetInstance().isSelectingObject = false; // Objeto seleccionado
							GuiLayer::GetInstance().showModelSelectionCombo = false;
						}
						else
						{
							std::cout << "No se ha seleccionado ningún objeto" << std::endl;
							GuiLayer::GetInstance().isSelectingObject = false;
							GuiLayer::GetInstance().showModelSelectionCombo = false;
							EntityManager::GetInstance().currentSelectedEntityInScene = entt::null; // Deselecciona la entidad
						}
					}
					else
					{
						// Trazas: Click fuera del Viewport
						std::cout << "Click detectado fuera del Viewport" << std::endl;
					}
				}
			}
		}







		

		//Con raycast y AABB
		//if (m_EngineMode == EngineMode::EDITOR_MODE && GuiLayer::GetInstance().mouseInsideViewport == true)
		//{
		//	//--MOUSE PICKING
		//	if (usingGizmo == false)
		//	{
		//		if (GuiLayer::GetInstance().isSelectingObject == true)
		//		{
		//			return;
		//		}

		//		GuiLayer::GetInstance().isSelectingObject = false;

		//		float mouseX, mouseY;
		//		std::tie(mouseX, mouseY) = InputManager::Instance().GetMousePosition();

		//		if (InputManager::Instance().IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT) && GuiLayer::GetInstance().isSelectingObject == false)
		//		{
		//			EntityManager::GetInstance().entitiesInRay.clear();

		//			float normalizedX = (2.0f * mouseX) / ViewportManager::GetInstance().viewports[0]->viewportSize.x - 1.0f;
		//			float normalizedY = ((2.0f * mouseY) / ViewportManager::GetInstance().viewports[0]->viewportSize.y - 1.0f) * -1.0f;

		//			glm::vec3 clipSpaceCoordinates(normalizedX, normalizedY, -1.0);
		//			glm::vec4 homogenousClipCoordinates = glm::vec4(clipSpaceCoordinates, 1.0);
		//			glm::mat4 invProjView = glm::inverse(ViewportManager::GetInstance().viewports[0]->camera->cameraMatrix);
		//			glm::vec4 homogenousWorldCoordinates = invProjView * homogenousClipCoordinates;
		//			glm::vec3 worldCoordinates = glm::vec3(homogenousWorldCoordinates) / homogenousWorldCoordinates.w;

		//			glm::vec3 rayOrigin = ViewportManager::GetInstance().viewports[0]->camera->Position;
		//			glm::vec3 rayDirection = glm::normalize(worldCoordinates - rayOrigin);

		//			EntityManager::GetInstance().CheckRayModelIntersection(rayOrigin, rayDirection);

		//			if (EntityManager::GetInstance().entitiesInRay.size() == 1)
		//			{
		//				EntityManager::GetInstance().currentSelectedEntityInScene = EntityManager::GetInstance().entitiesInRay[0];
		//				GuiLayer::GetInstance().isSelectingObject = false; // No need to select, auto-selected
		//				GuiLayer::GetInstance().showModelSelectionCombo = false;
		//			}
		//			else if (EntityManager::GetInstance().entitiesInRay.size() > 1) {
		//				GuiLayer::GetInstance().isSelectingObject = true; // Multiple options, need to select
		//				GuiLayer::GetInstance().showModelSelectionCombo = true;
		//				EntityManager::GetInstance().currentSelectedEntityInScene = entt::null;
		//			}
		//			else {
		//				GuiLayer::GetInstance().isSelectingObject = false; // No selection
		//				GuiLayer::GetInstance().showModelSelectionCombo = false;
		//				EntityManager::GetInstance().currentSelectedEntityInScene = entt::null;
		//				EntityManager::GetInstance().entitiesInRay.clear();
		//			}
		//		}
		//	}
		//}
		//-------------------------------------------
	}
	// -------------------------------------------------


    //--OTHERS
	void Engine::SetEngineState(EditorStates newState)
	{
		{
			if (m_CurrentState != newState) 
			{
				m_CurrentState = newState;
				EventManager::OnEngineStateChanged().trigger(m_CurrentState);
			}
		}
	}
    Timestep Engine::GetDeltaTime()
    {
        return m_deltaTime;
    }
	// -------------------------------------------------
}
