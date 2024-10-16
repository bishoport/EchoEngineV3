#pragma once

#include "../Core.h"
#include "GBO.hpp"
#include "FBO.hpp"
#include "../../tools/cameras/Camera.h"
#include "../../managers/EventManager.h"
#include <glm/glm.hpp>

namespace libCore
{
    class Viewport
    {
    public:

        bool resizingInProgress = false;

        // Nombre del viewport
        std::string viewportName = "no_name_viewport";
        int viewportNumber = 0;

        // Dimensiones del viewport
        glm::vec2 viewportSize = glm::vec2(800.0f, 600.0f);
        glm::vec2 viewportPos = glm::vec2(0.0f, 0.0f);
        glm::vec2 previousViewportSize = glm::vec2(800.0f, 600.0f);
        glm::vec2 previousViewportPos = glm::vec2(0.0f, 0.0f);

        // Estado del viewport
        bool isRenderable = true;
        bool isResizing = false;
        bool isMoving = false;
        bool mouseInviewport = false;

        // �nica c�mara para el viewport
        Ref<libCore::Camera> camera = nullptr;

        // Framebuffers (FBOs)
        Ref<libCore::FBO> framebuffer_shadowmap = nullptr;
        Ref<libCore::FBO> framebuffer_deferred = nullptr;
        Ref<libCore::FBO> framebuffer_forward = nullptr;
        Ref<libCore::FBO> framebuffer_final = nullptr;
        Ref<libCore::FBO> framebuffer_HDR = nullptr;
        Ref<libCore::FBO> framebuffer_SSAO = nullptr;
        Ref<libCore::FBO> framebuffer_SSAOBlur = nullptr;
        Ref<libCore::FBO> framebuffer_picking = nullptr;


        // Geometry Buffer (GBO)
        Ref<libCore::GBO> gBuffer = nullptr;

        // Constructor por defecto
        Viewport();

        // Constructor con nombre
        Viewport(const std::string& name);

        // M�todos de blit entre FBO y GBO
        void blitFBO2FBO(Ref<libCore::FBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter = GL_NEAREST);
        void blitGBO2FBO(Ref<libCore::GBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter = GL_NEAREST);

        // Desvincula todos los framebuffers, G-buffers y otros objetos OpenGL
        void UnbindAll();

        // Funci�n para revisar si los framebuffers y el G-buffer son v�lidos
        void CheckBuffers() const;

   // private:
        // Centralizar inicializaci�n y redimensionamiento de buffers
        void InitializeBuffers(int width, int height);

        // Actualiza el tama�o de la c�mara (usa la c�mara miembro)
        void UpdateCameraSizeView(const int width, const int height);

        // Actualiza la posici�n del viewport con la nueva posici�n de la ventana
        void UpdateViewportPosition(int xpos, int ypos);

        // Suscripciones a eventos de cambio de tama�o y posici�n
        void SetupEventSubscriptions();

        // Redimensiona los buffers y framebuffers
        void ResizeBuffers(int width, int height);

        // Funci�n auxiliar para verificar el estado de un framebuffer
        void CheckFramebuffer(const std::string& name, Ref<FBO> framebuffer) const;
    };
}
