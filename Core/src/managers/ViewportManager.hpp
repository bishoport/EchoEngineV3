#pragma once

#include "../core/Core.h"
#include "../core/renderer/Viewport.h"
#include "../core/renderer/FBO.hpp"
#include "../tools/cameras/EditorCamera.h"
#include "../tools/cameras/GameCamera.h"
#include "../core/renderer/Renderer.hpp"

namespace libCore
{
    class ViewportManager {

    public:

        static ViewportManager& GetInstance() {
            static ViewportManager instance;
            return instance;
        }

        std::vector<Ref<Viewport>> viewports;

        void CreateViewport(std::string name, glm::vec3 cameraPosition, int viewportWidth, int viewportHeight, CAMERA_CONTROLLERS controller)
        {
            auto viewport = CreateRef<Viewport>();
            viewport->viewportName = name;

            viewport->viewportSize = glm::vec2(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));
            //viewport->viewportSize.x = static_cast<float>(viewportWidth);
            //viewport->viewportSize.y = static_cast<float>(viewportHeight);

            // Cameras
            viewport->camera = CreateRef<libCore::EditorCamera>(viewport->viewportSize.x, viewport->viewportSize.y, cameraPosition);
            viewport->camera->SetPosition(glm::vec3(-20.0f, 8.0f, -20.0f));
            viewport->camera->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
            //----------------------------------------------------------

            // G-Buffer
            auto gbo = CreateRef<GBO>();
            gbo->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y));
            viewport->gBuffer = std::move(gbo);
            //----------------------------------------------------------



            // F-Buffer Deferred + lighting
            auto fbo0 = CreateRef<FBO>();
            fbo0->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_RGB16, "F-Buffer Deferred-lighting", true, false, false);
            fbo0->addAttachment("color", GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
            fbo0->addAttachment("depth", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo0->closeSetup();
            viewport->framebuffer_deferred = std::move(fbo0);
            //----------------------------------------------------------

            // F-Buffer Deferred + Forward 
            auto fbo1 = CreateRef<FBO>();
            fbo1->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_RGB8, "F-Buffer Deferred-Forward", true, false, false);
            fbo1->addAttachment("color", GL_RGB8, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
            fbo1->addAttachment("depth", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo1->closeSetup();
            viewport->framebuffer_forward = std::move(fbo1);
            //----------------------------------------------------------

            // F-Buffer Final
            auto fbo2 = CreateRef<FBO>();
            fbo2->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_RGB8, "F-Buffer Final", true, false, false);
            fbo2->addAttachment("color", GL_RGB8, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
            fbo2->addAttachment("depth", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo2->closeSetup();
            viewport->framebuffer_final = std::move(fbo2);
            //----------------------------------------------------------

            // F-Buffer HDR
            auto fbo3 = CreateRef<FBO>();
            fbo3->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_RGB16F, "F-Buffer HDR", true, false, false); // Este buffer tiene una coma flotante más grande porque es para HDR
            fbo3->addAttachment("color", GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
            fbo3->addAttachment("depth", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo3->closeSetup();
            viewport->framebuffer_HDR = std::move(fbo3);
            //----------------------------------------------------------

            // F-Buffer SSAO
            auto fbo4 = CreateRef<FBO>();
            fbo4->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_RED, "F-Buffer SSAO", true, false, false);
            fbo4->addAttachment("color", GL_RED, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);
            fbo4->addAttachment("depth", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo4->closeSetup();
            viewport->framebuffer_SSAO = std::move(fbo4);
            //----------------------------------------------------------

            // F-Buffer SSAO-BLUR
            auto fbo5 = CreateRef<FBO>();
            fbo5->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_RED, "F-Buffer SSAO-BLUR", true, false, false);
            fbo5->addAttachment("color", GL_RED, GL_RED, GL_FLOAT, GL_COLOR_ATTACHMENT0);
            fbo5->addAttachment("depth", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo5->closeSetup();
            viewport->framebuffer_SSAOBlur = std::move(fbo5);
            //----------------------------------------------------------

            // F-Buffer ShadowMap
            auto fbo6 = CreateRef<FBO>();
            fbo6->init(static_cast<int>(viewport->viewportSize.x), static_cast<int>(viewport->viewportSize.y), GL_DEPTH_COMPONENT, "F-Buffer ShadowMap", true, false, false);
            fbo6->addAttachment("depth", GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT);
            fbo6->closeSetup();
            viewport->framebuffer_shadowmap = std::move(fbo6);
            //----------------------------------------------------------

            // Add Viewport to collection
            viewports.push_back(std::move(viewport));
        }

        GLuint CubemapFaceTo2DTexture(GLuint cubemap, GLenum face, int width, int height, GLuint captureFBO)
        {
            GLuint texture2D;
            glGenTextures(1, &texture2D);
            glBindTexture(GL_TEXTURE_2D, texture2D);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, cubemap, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return texture2D;
        }

        private:
            ViewportManager() {};
    };
}


