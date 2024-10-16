#pragma once
#include "PanelBase.h"
#include "../../managers/SceneManager.h"
#include "../../managers/SceneHistoryManager.h"  // Agregar este include para el historial de escenas

namespace libCore
{
    class ScenePanel : public PanelBase
    {
    public:
        ScenePanel() : PanelBase("Scene") {}

        void Init() override
        {
            // Inicializa la escena actual si es necesario
            auto currentScene = SceneManager::GetInstance().GetCurrentScene();

            EventManager::OnEngineStateChanged().subscribe([this](EditorStates newState)
                {
                    switch (newState) 
                    {
                    case NONE:
                        currentEditorState = "NONE";
                        break;
                    case BUSY:
                        currentEditorState = "BUSY";
                        break;
                    case PREPARE_STOP:
                        currentEditorState = "PREPARE_STOP";
                        break;
                    case PAUSE:
                        currentEditorState = "PAUSE";
                        break;
                    case PREPARE_PLAY:
                        currentEditorState = "PREPARE_PLAY";
                        break;
                    case PLAY:
                        currentEditorState = "PLAY";
                        break;
                    case STOP:
                        currentEditorState = "STOP";
                        break;
                    }
                });
        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());

            ImGui::Text("EDITOR STATE %s", currentEditorState.c_str());
            ImGui::Spacing();
            ImGui::Spacing();

            //Ref<Viewport> viewport = ViewportManager::GetInstance().viewports[0];  // Asegúrate de que el índice es correcto
            //ImGui::Text("Panel Pos X: %.1f, Panel Pos Y: %.1f", viewport->viewportPos.x, viewport->viewportPos.y);
            //ImGui::Text("Panel Size W: %.1f, Panel Size H: %.1f", viewport->viewportSize.x, viewport->viewportSize.y);
            //ImGui::SliderInt("offset X", &Engine::GetInstance().offSetMouseX, -200, 200);
            //ImGui::SliderInt("offset Y", &Engine::GetInstance().offSetMouseY, -200, 200);
            //ImGui::Spacing();
            //ImGui::Spacing();

            //-- Obtener la escena actual
            auto currentScene = SceneManager::GetInstance().GetCurrentScene();

            //-- Buffer para almacenar el nuevo nombre
            static char sceneNameBuffer[128];
            strncpy(sceneNameBuffer, currentScene->sceneName.c_str(), sizeof(sceneNameBuffer));

            ImGui::Text("SAVE SCENE");
            //-- Mostrar y permitir la edición del nombre
            ImGui::Text("Current Scene Name: ");
            if (ImGui::InputText("##SceneName", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer))) {
                // Actualizar el nombre de la escena cuando se edita
                currentScene->sceneName = std::string(sceneNameBuffer);
            }

            // Botón para guardar la escena actual con icono Font Awesome de "Guardar"
            if (ImGui::Button(ICON_FA_SAVE " Save Scene")) {
                //SceneManager::GetInstance().SerializeSceneTask();
                SceneManager::GetInstance().SaveScene();
                // Añadir la escena guardada al historial
                SceneHistoryManager::GetInstance().AddSceneToHistory(currentScene->sceneName);
            }

            ImGui::Spacing();
            ImGui::Spacing(); 
            ImGui::Spacing();

            ImGui::Text("LOAD SCENE");
            // Obtener el historial de escenas
            const auto& sceneHistory = SceneHistoryManager::GetInstance().GetSceneHistory();

            // Lista de selección para cargar una escena del historial
            static int selectedSceneIndex = -1;
            if (ImGui::BeginCombo("Scene History", (selectedSceneIndex >= 0) ? sceneHistory[selectedSceneIndex].c_str() : "Select Scene")) {
                for (int i = 0; i < sceneHistory.size(); ++i) {
                    bool isSelected = (selectedSceneIndex == i);
                    if (ImGui::Selectable(sceneHistory[i].c_str(), isSelected)) {
                        selectedSceneIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Botón para cargar la escena seleccionada con icono Font Awesome de "Cargar"
            if (selectedSceneIndex >= 0 && ImGui::Button(ICON_FA_FOLDER_OPEN " Load Scene")) {
                std::string selectedScene = sceneHistory[selectedSceneIndex];
                SceneManager::GetInstance().LoadScene(selectedScene);
                //SceneManager::GetInstance().LoadSceneTask(selectedScene);
            }
            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }

    private:
        std::string currentEditorState = "NONE";
    };
}
