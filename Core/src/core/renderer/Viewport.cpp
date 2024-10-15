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
            std::cerr << "Error: Intento de inicializar framebuffers con dimensiones inv�lidas ("<< width << "x" << height << ")." << std::endl;
            return;
        }

        // Si las dimensiones son v�lidas, redimensiona los framebuffers
        if (gBuffer != nullptr) gBuffer->resize(width, height);
        
        if (framebuffer_deferred != nullptr) framebuffer_deferred->resize(width, height);
        if (framebuffer_shadowmap != nullptr) framebuffer_shadowmap->resize(width, height);
        if (framebuffer_forward != nullptr) framebuffer_forward->resize(width, height);
        
        if (framebuffer_HDR != nullptr) framebuffer_HDR->resize(width, height);
        if (framebuffer_SSAO != nullptr) framebuffer_SSAO->resize(width, height);
        if (framebuffer_SSAOBlur != nullptr) framebuffer_SSAOBlur->resize(width, height);
        if (framebuffer_picking != nullptr) framebuffer_picking->resize(width, height);

        if (framebuffer_final != nullptr) framebuffer_final->resize(width, height);
    }

    void Viewport::SetupEventSubscriptions()
    {
        // Suscribirse a eventos de cambio de tama�o del panel
        EventManager::OnPanelResizedEvent().subscribe([this](const std::string& name, const glm::vec2& size, const glm::vec2& position)
            {
                // Actualiza la posici�n y tama�o del Viewport
                viewportSize = size;
                viewportPos = position;  // Esta posici�n ahora ser� la posici�n global corregida desde el panel

                // Redimensionar los framebuffers
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
        // Guardar el framebuffer actual
        GLint prevReadFramebuffer, prevDrawFramebuffer;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFramebuffer);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFramebuffer);

        // Vincular los framebuffers de origen y destino
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->getFramebuffer());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());

        // Realizar el blit
        glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, target->getWidth(), target->getHeight(), mask, filter);

        // Restaurar el framebuffer anterior
        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFramebuffer);

        // Verificar si la operaci�n de blit fue exitosa
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Blit operation failed!" << std::endl;
        }
    }

    // Blit entre GBO y FBO
    void Viewport::blitGBO2FBO(Ref<libCore::GBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter)
    {
        // Guardar el framebuffer actual
        GLint prevReadFramebuffer, prevDrawFramebuffer;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFramebuffer);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFramebuffer);

        // Vincular el GBO como framebuffer de lectura y el FBO como framebuffer de escritura
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());

        // Realizar el blit
        glBlitFramebuffer(0, 0, src->bufferWidth, src->bufferHeight, 0, 0, target->getWidth(), target->getHeight(), mask, filter);

        // Restaurar el framebuffer anterior
        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFramebuffer);

        // Verificar si la operaci�n de blit fue exitosa
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Blit operation failed!" << std::endl;
        }
    }




    void Viewport::UnbindAll()
    {
        // Desvincular cualquier framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Desvincular cualquier textura
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        // Desvincular cualquier G-buffer si es necesario
        if (gBuffer != nullptr) {
            gBuffer->unbindGBuffer(); // Aseg�rate de tener una funci�n unbind() en tu clase GBO
        }

        // Desvincular FBOs (Framebuffers)
        if (framebuffer_shadowmap != nullptr) framebuffer_shadowmap->unbindFBO();
        if (framebuffer_deferred != nullptr) framebuffer_deferred->unbindFBO();
        if (framebuffer_forward != nullptr) framebuffer_forward->unbindFBO();
        if (framebuffer_final != nullptr) framebuffer_final->unbindFBO();
        if (framebuffer_HDR != nullptr) framebuffer_HDR->unbindFBO();
        if (framebuffer_SSAO != nullptr) framebuffer_SSAO->unbindFBO();
        if (framebuffer_SSAOBlur != nullptr) framebuffer_SSAOBlur->unbindFBO();
        if (framebuffer_picking != nullptr) framebuffer_picking->unbindFBO();

        // Si es necesario, puedes agregar otras desvinculaciones aqu� (ej. shaders, VAO, etc.)
        glUseProgram(0);  // Desvincular cualquier programa de shaders activo, si aplica
        glBindVertexArray(0);  // Desvincular cualquier VAO activo
    }



    void Viewport::CheckBuffers() const
    {
        // Verificar si el GBuffer es v�lido y comprobar sus texturas
        if (gBuffer != nullptr) {
            std::cout << "Checking GBuffer..." << std::endl;
            gBuffer->checkTextures();  // Asume que `GBO` tiene una funci�n `checkTextures`
        }
        else {
            std::cerr << "ERROR: GBuffer is nullptr." << std::endl;
        }

        // Verificar cada framebuffer si es v�lido y si su estado es correcto
        CheckFramebuffer("Shadow Map Framebuffer", framebuffer_shadowmap);
        CheckFramebuffer("Deferred Framebuffer", framebuffer_deferred);
        CheckFramebuffer("Forward Framebuffer", framebuffer_forward);
        CheckFramebuffer("Final Framebuffer", framebuffer_final);
        CheckFramebuffer("HDR Framebuffer", framebuffer_HDR);
        CheckFramebuffer("SSAO Framebuffer", framebuffer_SSAO);
        CheckFramebuffer("SSAO Blur Framebuffer", framebuffer_SSAOBlur);
        CheckFramebuffer("Picking Framebuffer", framebuffer_picking);
    }

    // Funci�n auxiliar para revisar un framebuffer
    void Viewport::CheckFramebuffer(const std::string& name, Ref<FBO> framebuffer) const
    {
        if (framebuffer != nullptr) {
            framebuffer->bindFBO();
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status == GL_FRAMEBUFFER_COMPLETE) {
                std::cout << name << " is complete and valid." << std::endl;
            }
            else {
                std::cerr << "ERROR: " << name << " is not complete. Status: " << status << std::endl;
            }
            framebuffer->unbindFBO();
        }
        else {
            std::cerr << "ERROR: " << name << " is nullptr." << std::endl;
        }
    }
}





////Tambien quiero que asegures las funciones blitFBO2FBO y blitGBO2FBO, que siempre
//// Blit entre Framebuffers
//void Viewport::blitFBO2FBO(Ref<libCore::FBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter)
//{
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->getFramebuffer());
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());
//    glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, target->getWidth(), target->getHeight(), mask, filter);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}

//// Blit entre GBO y FBO
//void Viewport::blitGBO2FBO(Ref<libCore::GBO> src, Ref<libCore::FBO> target, GLbitfield mask, GLenum filter)
//{
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->gBuffer);
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->getFramebuffer());
//    glBlitFramebuffer(0, 0, src->bufferWidth, src->bufferHeight, 0, 0, target->getWidth(), target->getHeight(), mask, filter);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}