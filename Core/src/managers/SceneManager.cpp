#include "SceneManager.h"

#include "../core/Engine.h"

#include "../managers/EntityManager.h"
#include "../managers/EventManager.h"
#include "../managers/BackgroundTaskManager.h"

namespace libCore
{

    void SceneManager::SetupSceneManager()
    {
        EventManager::OnEngineStateChanged().subscribe([this](EditorStates newState) 
        {
            switch (newState) {
            case NONE:
                std::cout << "Editor state: NONE\n";
                break;
            case BUSY:
                std::cout << "Editor state: BUSY\n";
                break;
            case PREPARE_STOP:
                std::cout << "Editor state: PREPARE_STOP\n";
                EntityManager::GetInstance().runScripting = false;
                EntityManager::GetInstance().CheckInstancesInRunTime();
                DeserializeComponentsSceneTask();
                //LoadComponentsScene();
                break;
            case STOP:
                std::cout << "Editor state: STOP\n";
                //EntityManager::GetInstance().runScripting = false;
                break;
            case PAUSE:
                std::cout << "Editor state: PAUSE\n";
                //EntityManager::GetInstance().runScripting = false;
                break;
            case PREPARE_PLAY:
                std::cout << "Editor state: PREPARE_PLAY\n";
                //currentScene->SerializeComponents();
                SerializeComponentsSceneTask();
                //SaveComponentsScene();
                break;
            case PLAY:
                std::cout << "Editor state: PLAY\n";
                EntityManager::GetInstance().InitScripts();
                EntityManager::GetInstance().runScripting = true;
                break;
            }
        });
    }

    //--SCENE
    void SceneManager::CreateNewScene(const std::string& sceneName)
    {
        currentScene = CreateRef<Scene>(sceneName);
    }
    void SceneManager::ClearCurrentScene()
    {
        currentScene.reset();
    }
    //------------------------------------------------------------------------


    //--SAVE/LOAD public
    void SceneManager::SaveScene()
    {
        if (currentScene != nullptr)
        {
            currentScene->SerializeScene();
        }
    }
    void SceneManager::LoadScene(std::string _sceneName)
    {
        currentScene->DeserializeScene(_sceneName);
    }
    void SceneManager::SaveComponentsScene()
    {
        currentScene->SerializeComponents();
    }
    void SceneManager::LoadComponentsScene()
    {
        currentScene->DeserializeComponents();
    }
    
    //WITH TASK
    void SceneManager::SerializeSceneTask() {
        BackgroundTaskManager::GetInstance().AddTask(
            BackgroundTaskManager::Task(
                []() {  // startJob (se ejecuta en un hilo secundario)
                    SceneManager::GetInstance().SaveScene();  // Llamada a la función de guardado
                    MainThreadTaskManager::GetInstance().AddTask([]() {
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_INFO, "Saving scene...");
                        Engine::GetInstance().SetEngineState(BUSY);
                    });
                },
                nullptr,  // progressJob es opcional y no es necesario en este caso
                []() {  // endJob (se ejecuta en el hilo principal)
                    // Añadir una tarea en el hilo principal
                    MainThreadTaskManager::GetInstance().AddTask([]() 
                    {
                        Engine::GetInstance().SetEngineState(STOP);
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Save complete!");
                    });
                }
            )
        );
    }
    void SceneManager::LoadSceneTask(const std::string& sceneName) {
        BackgroundTaskManager::GetInstance().AddTask(
            BackgroundTaskManager::Task(
                [sceneName]() {  // startJob (se ejecuta en un hilo secundario)
                    SceneManager::GetInstance().LoadScene(sceneName);  // Llamada a la función de carga de escena
                    MainThreadTaskManager::GetInstance().AddTask([]() {
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_INFO, "Loading scene...");
                        Engine::GetInstance().SetEngineState(BUSY);  // Cambiar estado del motor
                        });
                },
                nullptr,  // progressJob es opcional y no es necesario en este caso
                [sceneName]() {  // endJob (se ejecuta en el hilo principal)
                    MainThreadTaskManager::GetInstance().AddTask([sceneName]() {
                        Engine::GetInstance().SetEngineState(STOP);  // Cambiar estado del motor cuando termine
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Scene '" + sceneName + "' loaded successfully!");
                        });
                }
            )
        );
    }
    void SceneManager::SerializeComponentsSceneTask() {
        BackgroundTaskManager::GetInstance().AddTask(
            BackgroundTaskManager::Task(
                []() {  // startJob (se ejecuta en un hilo secundario)
                    SceneManager::GetInstance().SaveComponentsScene();  // Llamada a la función de guardado
                    MainThreadTaskManager::GetInstance().AddTask([]() {
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_INFO, "Saving components...");
                        Engine::GetInstance().SetEngineState(BUSY);  // Cambiar estado del motor
                        });
                },
                nullptr,  // progressJob es opcional y no es necesario en este caso
                []() {  // endJob (se ejecuta en el hilo principal)
                    MainThreadTaskManager::GetInstance().AddTask([]() {
                        Engine::GetInstance().SetEngineState(PLAY);  // Cambiar estado del motor cuando termine
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Components saved successfully!");
                        });
                }
            )
        );
    }
    void SceneManager::DeserializeComponentsSceneTask() {
        BackgroundTaskManager::GetInstance().AddTask(
            BackgroundTaskManager::Task(
                []() {  // startJob (se ejecuta en un hilo secundario)
                    SceneManager::GetInstance().LoadComponentsScene();  // Llamada a la función de carga
                    MainThreadTaskManager::GetInstance().AddTask([]() {
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_INFO, "Loading components...");
                        Engine::GetInstance().SetEngineState(BUSY);  // Cambiar estado del motor
                        });
                },
                nullptr,  // progressJob es opcional y no es necesario en este caso
                []() {  // endJob (se ejecuta en el hilo principal)
                    MainThreadTaskManager::GetInstance().AddTask([]() {
                        Engine::GetInstance().SetEngineState(STOP);  // Cambiar estado del motor cuando termine
                        ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Components loaded successfully!");
                        });
                }
            )
        );
    }
    //------------------------------------------------------------------------


}
