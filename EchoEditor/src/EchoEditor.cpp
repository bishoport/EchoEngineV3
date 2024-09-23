#include "EchoEditor.h"
#include <core/Engine.h>

void EchoEditor::Init()
{
    // -- PREPARE ENGINE
    bool ok = libCore::Engine::GetInstance().InitializeEngine("MAP EDITOR", screenWidth, screenHeight , false);
    if (!ok) return;
    //-----------------------------------------------------------------

    // -- START LOOP OpenGL
    libCore::Engine::GetInstance().InitializeMainLoop();
    //------------------------------------------------------------------

}
