#pragma once

#include "Core.h"
#include "Timestep.h"

namespace libCore
{
    enum EngineStates
    {
        THINKING,
        EDITOR,
        FULL_PLAY
    };


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

        Timestep GetDeltaTime();

        bool usingGizmo = false;
    private:
        //ENGINE LIFE--CYCLE
        EngineStates engineState = EngineStates::EDITOR;
        bool running = false;
        Timestep m_deltaTime = 0.0f;
    };

}