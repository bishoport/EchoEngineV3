#pragma once

#include "../Core.h"


namespace libCore {

    class FBO {
    public:
        std::string nameFBO = "";

        FBO() : framebuffer(0), fboWidth(0), fboHeight(0), internalFormat(GL_RGB8),
            enableDepth(false), enableStencil(false), enableRBO(false) {}

        ~FBO() {
            cleanup();
        }

        void init(int width, int height, GLenum internalFormat, std::string name,
            bool enableDepth = false, bool enableStencil = false, bool enableRBO = false) {
            // Validar dimensiones antes de continuar
            if (width <= 0 || height <= 0) {
                std::cerr << "ERROR: Framebuffer dimensions must be greater than zero. Width: " << width << ", Height: " << height << std::endl;
                return;
            }

            nameFBO = name;
            fboWidth = width;
            fboHeight = height;
            this->internalFormat = internalFormat;
            this->enableDepth = enableDepth;
            this->enableStencil = enableStencil;
            this->enableRBO = enableRBO;

            glGenFramebuffers(1, &framebuffer);

            if (framebuffer == 0) {
                std::cerr << "ERROR: Failed to generate framebuffer." << std::endl;
                return;
            }

            bindFBO();

            if (enableRBO) {
                setupRBO();
            }
        }

        void addAttachment(const std::string& name, GLenum internalFormat, GLenum format, GLenum type, GLenum attachmentType = GL_COLOR_ATTACHMENT0) {
            // Comprobación de las dimensiones del framebuffer antes de añadir un attachment
            if (fboWidth <= 0 || fboHeight <= 0) {
                std::cerr << "ERROR: Framebuffer dimensions invalid for attachment. Width: " << fboWidth << ", Height: " << fboHeight << std::endl;
                return;
            }

            GLuint texture;
            glGenTextures(1, &texture);
            if (texture == 0) {
                std::cerr << "ERROR: Failed to generate texture for attachment: " << name << std::endl;
                return;
            }

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, fboWidth, fboHeight, 0, format, type, nullptr);
            setTextureParameters(attachmentType);

            if (glGetError() != GL_NO_ERROR) {
                std::cerr << "ERROR: Failed to create texture for attachment: " << name << std::endl;
                return;
            }

            // Verificación adicional del framebuffer después de agregar el attachment
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, texture, 0);
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR: Framebuffer not complete after adding attachment: " << name << std::endl;
            }

            attachments[name] = texture;

            if (attachmentType != GL_DEPTH_ATTACHMENT && attachmentType != GL_STENCIL_ATTACHMENT && attachmentType != GL_DEPTH_STENCIL_ATTACHMENT) {
                drawBuffers.push_back(attachmentType);
            }
        }

        void closeSetup() {
            if (!enableRBO && !drawBuffers.empty()) {
                glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
            }
            else {
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
            }
            unbindFBO();
        }

        void bindFBO() const {
            if (framebuffer == 0) {
                std::cerr << "ERROR: Attempted to bind an invalid framebuffer." << std::endl;
                return;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        }

        void unbindFBO() const {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }


        void bindTexture(const std::string& attachmentType, int textureSlot) const {
            // Comprobar si el attachmentType existe en el mapa de attachments
            auto it = attachments.find(attachmentType);
            if (it != attachments.end()) {
                glActiveTexture(GL_TEXTURE0 + textureSlot);
                glBindTexture(GL_TEXTURE_2D, it->second);
            }
            else {
                // Mostrar un error si no se encuentra el attachment
                std::cerr << "ERROR: Attachment type \"" << attachmentType << "\" not found in attachments." << std::endl;
            }
        }



        GLuint getTexture(const std::string& attachmentType) const {
            return attachments.at(attachmentType);
        }

        GLuint getFramebuffer() const {
            return framebuffer;
        }

        int getWidth() const {
            return fboWidth;
        }

        int getHeight() const {
            return fboHeight;
        }

        void resize(int newWidth, int newHeight) {
            // Validar las nuevas dimensiones antes de cambiar el tamaño
            if (newWidth <= 0 || newHeight <= 0) {
                std::cerr << "ERROR: Invalid dimensions for framebuffer resize. Width: " << newWidth << ", Height: " << newHeight << std::endl;
                return;
            }

            fboWidth = newWidth;
            fboHeight = newHeight;

            bindFBO();
            for (const auto& attachment : attachments) {
                glBindTexture(GL_TEXTURE_2D, attachment.second);
                if (attachment.first == "depth") {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, fboWidth, fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                }
                else {
                    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, fboWidth, fboHeight, 0, GL_RGB, GL_FLOAT, nullptr);
                }

                GLenum error = glGetError();
                if (error != GL_NO_ERROR) {
                    std::cerr << "ERROR: Failed to resize texture: " << attachment.first << ". OpenGL error: " << error << std::endl;
                }
            }
            unbindFBO();
        }


    private:
        GLuint framebuffer;
        std::map<std::string, GLuint> attachments;
        std::vector<GLuint> colorAttachments;
        std::vector<GLenum> drawBuffers;
        int fboWidth, fboHeight;
        GLenum internalFormat;
        bool enableDepth;
        bool enableStencil;
        bool enableRBO;

        GLuint rboDepthId;
        GLuint rboStencilId;

        void setTextureParameters(GLenum attachmentType) const {
            if (attachmentType == GL_DEPTH_ATTACHMENT) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }

        void setupRBO() {
            if (enableDepth) {
                glGenRenderbuffers(1, &rboDepthId);
                glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fboWidth, fboHeight);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);  // Desvincular renderbuffer

            }

            if (enableStencil) {
                glGenRenderbuffers(1, &rboStencilId);
                glBindRenderbuffer(GL_RENDERBUFFER, rboStencilId);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fboWidth, fboHeight);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboStencilId);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);  // Desvincular renderbuffer

            }
        }

        void cleanup() {
            for (const auto& attachment : attachments) {
                glDeleteTextures(1, &attachment.second);
            }
            if (framebuffer != 0) {
                glDeleteFramebuffers(1, &framebuffer);
                framebuffer = 0;
            }
            if (rboDepthId != 0) {
                glDeleteRenderbuffers(1, &rboDepthId);
                rboDepthId = 0;
            }
            if (rboStencilId != 0) {
                glDeleteRenderbuffers(1, &rboStencilId);
                rboStencilId = 0;
            }
        }

        void checkFBOStatus() const {
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete: ";
                switch (status) {
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    std::cerr << "Incomplete Attachment." << std::endl;
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    std::cerr << "Missing Attachment." << std::endl;
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    std::cerr << "Unsupported Framebuffer." << std::endl;
                    break;
                default:
                    std::cerr << "Unknown error." << std::endl;
                }
            }
        }

    };
}
