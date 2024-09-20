#pragma once

#include "../core/Core.h"
#include "../core/Timestep.h"

namespace libCore {

    enum SceneStates
    {
        NONE,
        THINKING,
        STOP,
        PLAY,
        PAUSE
    };

    class Scene {

    public:
        // Nombre de la escena
        std::string name;

        // Constructor que recibe el nombre de la escena
        Scene(const std::string& sceneName);

        //--SCENE LIFE-CYCLE
        void Init();
        void Update(Timestep m_deltaTime);
        void Render(Timestep m_deltaTime);



    };
}
