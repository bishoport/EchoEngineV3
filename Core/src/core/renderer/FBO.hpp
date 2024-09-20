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
            nameFBO = name;
            fboWidth = width;
            fboHeight = height;
            this->internalFormat = internalFormat;
            this->enableDepth = enableDepth;
            this->enableStencil = enableStencil;
            this->enableRBO = enableRBO;

            glGenFramebuffers(1, &framebuffer);
            bindFBO();

            if (enableRBO) {
                setupRBO();
            }
        }

        void addAttachment(const std::string& name, GLenum internalFormat, GLenum format, GLenum type, GLenum attachmentType = GL_COLOR_ATTACHMENT0) {
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, fboWidth, fboHeight, 0, format, type, nullptr);
            setTextureParameters(attachmentType);

            if (attachmentType == GL_COLOR_ATTACHMENT0) {
                colorAttachments.push_back(texture);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, texture, 0);
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
            checkFBOStatus();
            unbindFBO();
        }

        void bindFBO() const {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        }

        void unbindFBO() const {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void bindTexture(const std::string& attachmentType, int textureSlot) const {
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(GL_TEXTURE_2D, attachments.at(attachmentType));
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
            }

            if (enableStencil) {
                glGenRenderbuffers(1, &rboStencilId);
                glBindRenderbuffer(GL_RENDERBUFFER, rboStencilId);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fboWidth, fboHeight);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboStencilId);
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
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            }
        }
    };
}
