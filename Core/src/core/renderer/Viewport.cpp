#include "Viewport.h"

namespace libCore
{
    Viewport::Viewport()
    {
        SetupEventSubscriptions();
    }

    Viewport::Viewport(const std::string& name) : viewportName(name)
    {
        SetupEventSubscriptions();
    }




    // Funci�n para centralizar la inicializaci�n de buffers
    void Viewport::InitializeBuffers(int width, int height)
    {
        // Verifica que el ancho y alto sean mayores que 0
        if (width <= 0 || height <= 0)
        {
            std::cerr << "Error: Intento de inicializar framebuffers con dimensiones inv�lidas ("
                << width << "x" << height << ")." << std::endl;
            return;
        }

        // Si las dimensiones son v�lidas, redimensiona los framebuffers
        gBuffer->resize(width, height);
        framebuffer_deferred->resize(width, height);
        framebuffer_shadowmap->resize(width, height);
        framebuffer_forward->resize(width, height);
        framebuffer_final->resize(width, height);
        framebuffer_HDR->resize(width, height);
        framebuffer_SSAO->resize(width, height);
        framebuffer_SSAOBlur->resize(width, height);
    }

    void Viewport::SetupEventSubscriptions()
    {
        // Suscribirse a eventos de cambio de tama�o del panel
        EventManager::OnPanelResizedEvent().subscribe([this](const std::string& name, const glm::vec2& size, const glm::vec2& position)
            {
                viewportSize = size;
                viewportPos = glm::vec2(position.x, viewportSize.y - position.y);

                // Redimensionar todos los framebuffers y la GBO de manera centralizada
                ResizeBuffers(static_cast<int>(size.x), static_cast<int>(size.y));

                // Actualizar el tama�o de la c�mara
                UpdateCameraSizeView(static_cast<int>(size.x), static_cast<int>(size.y));
            });

        // Suscribirse a eventos de cambio de tama�o de la ventana
        EventManager::OnWindowResizeEvent().subscribe([this](const int width, const int height)
            {
                viewportSize = glm::vec2(static_cast<float>(width), static_cast<float>(height));

                // Redimensionar buffers de manera centralizada
                ResizeBuffers(width, height);

                // Actualizar el tama�o de la c�mara
                UpdateCameraSizeView(width, height);
            });

        // Suscribirse a eventos de cambio de posici�n de la ventana
        EventManager::OnWindowMovedEvent().subscribe([this](const int xpos, const int ypos)
            {
                // Actualizamos la posici�n del Viewport considerando la nueva posici�n de la ventana
                UpdateViewportPosition(xpos, ypos);
            });
    }

    // Funci�n centralizada para redimensionar los buffers y framebuffers
    void Viewport::ResizeBuffers(int width, int height)
    {
        // Verifica que las dimensiones sean v�lidas antes de redimensionar
        if (width > 0 && height > 0)
        {
            resizingInProgress = true;
            InitializeBuffers(width, height);  // Centralizamos el redimensionamiento
            resizingInProgress = false;
        }
        else
        {
            std::cerr << "Error: Se intent� redimensionar framebuffers con dimensiones inv�lidas ("
                << width << "x" << height << ")." << std::endl;
        }
    }

    // Funci�n para actualizar la posici�n del viewport y la c�mara
    void Viewport::UpdateViewportPosition(int xpos, int ypos)
    {
        // Actualizamos la posici�n de la ventana GLFW en el Viewport
        viewportPos = glm::vec2(xpos, ypos);
    }

    void Viewport::UpdateCameraSizeView(const int width, const int height)
    {
        if (camera != nullptr && width > 0 && height > 0)
        {
            camera->width = width;
            camera->height = height;
            camera->updateMatrix();
        }
        else
        {
            std::cerr << "Error: Intento de actualizar la c�mara con dimensiones inv�lidas o c�mara nula." << std::endl;
        }
    }

    // Blit entre Framebuffers
    void Viewport::blitFBO2FBO(Ref<libCore::FBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->getFramebuffer());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());
        glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, target->getWidth(), target->getHeight(), mask, filter);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Blit entre GBO y FBO
    void Viewport::blitGBO2FBO(Ref<libCore::GBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());
        glBlitFramebuffer(0, 0, src->bufferWidth, src->bufferHeight, 0, 0, target->getWidth(), target->getHeight(), mask, filter);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
