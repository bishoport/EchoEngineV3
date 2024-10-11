#pragma once

#include "Core.h"
#include "Timestep.h"

namespace libCore
{
    class Engine
    {
    public:
        static Engine& GetInstance()
        {
            static Engine instance;
            return instance;
        }

        bool InitializeEngine(const std::string& windowTitle, int initialWindowWidth, int initialWindowHeight , bool full_play = false);
        void InitializeMainLoop();
        void StopMainLoop();

        void UpdateBeforeRender();

        void SetEngineState(EditorStates newState);

        EditorStates GetEngineState() const {
            return m_CurrentState;
        }
        Timestep GetDeltaTime();

        bool usingGizmo = false;

        int offSetMouseX = 0;
        int offSetMouseY = -200;

    private:
        //ENGINE LIFE--CYCLE
        EngineMode   m_EngineMode = EngineMode::EDITOR_MODE;
        EditorStates m_CurrentState = EditorStates::NONE;
        bool running = false;
        Timestep m_deltaTime = 0.0f;
    };

}