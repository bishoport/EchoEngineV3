#pragma once
#include "Camera.h"

namespace libCore
{
    //----------------------------------ATENCION-------------------------------------------------------------------
    //--Esta clase solo genera un controlador sin comportamiento, porque será controlada por un componente Script--
    //-------------------------------------------------------------------------------------------------------------

    class GameCamera : public Camera
    {
    public:
        GameCamera();
        void Inputs(libCore::Timestep deltaTime) override;
    };
    inline GameCamera::GameCamera() : Camera(800, 600, glm::vec3(0.0f, 0.0f, 0.0f)){}
    inline void GameCamera::Inputs(libCore::Timestep deltaTime){}
}


