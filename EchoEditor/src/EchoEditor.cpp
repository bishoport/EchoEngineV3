#include "EchoEditor.h"

#include <core/Engine.h>

void EchoEditor::Init()
{
    // -- PREPARE ENGINE
    bool ok = libCore::Engine::GetInstance().InitializeEngine("MAP EDITOR", screenWidth, screenHeight);
    //bool ok = libCore::Engine::GetInstance().InitializeEngine("MAP EDITOR", screenWidth, screenHeight,true);
    if (!ok) return;
    //-----------------------------------------------------------------

    // -- VIEWPORTS
    //libCore::Engine::GetInstance().CreateViewport("EDITOR CAMERA", glm::vec3(0.0f, 20.0f, 0.0f), CAMERA_CONTROLLERS::EDITOR);
    //------------------------------------------------------------------

    // -- START LOOP OpenGL
    libCore::Engine::GetInstance().InitializeMainLoop();
    //------------------------------------------------------------------

}
