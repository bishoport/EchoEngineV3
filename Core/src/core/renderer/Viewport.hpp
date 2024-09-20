//#pragma once
//#include "../Core.h"
//#include "GBO.hpp"
//#include "FBO.hpp"
//#include "../../tools/cameras/Camera.h"
//#include "../../managers/EventManager.h"
//#include <imgui.h>
//
//namespace libCore
//{
//    class Viewport
//    {
//    public:
//
//        //--ID
//        std::string viewportName = "no_name_viewport";
//
//        //--DIMENSIONS
//        ImVec2 viewportSize         = ImVec2(800.0f, 600.0f);
//        ImVec2 viewportPos          = ImVec2(0.0f, 0.0f);
//        ImVec2 previousViewportSize = ImVec2(800.0f, 600.0f);
//        ImVec2 previousViewportPos  = ImVec2(0.0f, 0.0f);
//
//        //--BEHAVIOR
//        bool isResizing      = false;
//        bool isMoving        = false;
//        bool mouseInviewport = false;
//
//        //--Camera
//        Ref<libCore::Camera> camera = nullptr;
//
//        Ref<libCore::Camera> editorCamera = nullptr;
//        Ref<libCore::Camera> gameCamera = nullptr;
//
//        
//        //--FBO´s
//        Ref<libCore::FBO> framebuffer_shadowmap = nullptr;
//        Ref<libCore::FBO> framebuffer_deferred  = nullptr;
//        Ref<libCore::FBO> framebuffer_forward   = nullptr;
//        Ref<libCore::FBO> framebuffer_final     = nullptr;
//        Ref<libCore::FBO> framebuffer_HDR       = nullptr;
//        Ref<libCore::FBO> framebuffer_SSAO      = nullptr;
//        Ref<libCore::FBO> framebuffer_SSAOBlur  = nullptr;
//
//        //--GBO
//        Ref<libCore::GBO> gBuffer = nullptr;
//
//        //--BLIT´s
//        void blitFBO2FBO(Ref<libCore::FBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter = GL_NEAREST) {
//            glBindFramebuffer(GL_READ_FRAMEBUFFER, src->getFramebuffer());
//            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());
//            glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, target->getWidth(), target->getHeight(), mask, filter);
//            glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        }
//        void blitGBO2FBO(Ref<libCore::GBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter = GL_NEAREST) {
//            glBindFramebuffer(GL_READ_FRAMEBUFFER, src->gBuffer);
//            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());
//            glBlitFramebuffer(0, 0, src->bufferWidth, src->bufferHeight, 0, 0, target->getWidth(), target->getHeight(), mask, filter);
//            glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        }
//
//        //--CONSTRUCTORS
//        Viewport()
//        {
//            //--RESIZE EVENT´s
//            EventManager::OnPanelResizedEvent().subscribe([this](const std::string name, const glm::vec2& size, const glm::vec2& position)
//            {
//                viewportSize.x = size.x;
//                viewportSize.y = size.y;
//
//                viewportPos.x = position.x;
//                viewportPos.y = viewportSize.y - position.y;
//
//                gBuffer->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//
//                framebuffer_deferred->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//                framebuffer_shadowmap->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//                framebuffer_forward->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//                framebuffer_final->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//                framebuffer_HDR->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//                framebuffer_SSAO->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//                framebuffer_SSAOBlur->resize(static_cast<int>(size.x), static_cast<int>(size.y));
//
//                UpdateCameraSizeView(editorCamera, static_cast<int>(size.x), static_cast<int>(size.y));
//                UpdateCameraSizeView(gameCamera, static_cast<int>(size.x), static_cast<int>(size.y)); 
//            });
//
//            EventManager::OnWindowResizeEvent().subscribe([this](const int width, const int height)
//            {
//                viewportSize.x = static_cast<float>(width);
//                viewportSize.y = static_cast<float>(height);
//
//                gBuffer->resize(width, height);
//
//                framebuffer_deferred->resize(width, height);
//                framebuffer_shadowmap->resize(width, height);
//                framebuffer_forward->resize(width, height);
//                framebuffer_final->resize(width, height);
//                framebuffer_HDR->resize(width, height);
//                framebuffer_SSAO->resize(width, height);
//                framebuffer_SSAOBlur->resize(width, height);
//
//                UpdateCameraSizeView(editorCamera, width, height);
//                UpdateCameraSizeView(gameCamera, width, height);
//            });
//            //---------------------------------------------------
//        }
//
//        void UpdateCameraSizeView(Ref<Camera> _camera, const int width, const int height)
//        {
//            if (_camera != nullptr)
//            {
//                _camera->width = static_cast<int>(width);
//                _camera->height = static_cast<int>(height);
//            }
//        }
//
//        Viewport(const std::string& name) :viewportName(name), viewportSize(0, 0), viewportPos(0, 0), previousViewportSize(0, 0), previousViewportPos(0, 0), mouseInviewport(false) {}
//    };
//}
