#pragma once
#include "Camera.h"

namespace libCore
{
    class EditorCamera : public Camera
    {
    public:
        bool firstClick = true;

        EditorCamera(int width, int height, glm::vec3 position);

        void Inputs(libCore::Timestep deltaTime) override;
    };
}
