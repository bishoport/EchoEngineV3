#pragma once

#include "../Core.h"

namespace libCore {

    class GBO {

    public:

        GBO() : gBuffer(0), bufferWidth(0), bufferHeight(0) {}

        ~GBO() {
            cleanup();
        }

        void init(int width, int height) {
            bufferWidth = width;
            bufferHeight = height;
            setupGBuffer();
        }

        void bindGBuffer() {
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        }

        void unbindGBuffer() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void bindTexture(const std::string& attachmentType, int textureSlot) {
            auto it = attachments.find(attachmentType);
            if (it != attachments.end()) {
                glActiveTexture(GL_TEXTURE0 + textureSlot);
                glBindTexture(GL_TEXTURE_2D, it->second);
            }
            else {
                std::cerr << "ERROR::GBO::bindTexture - Attachment type " << attachmentType << " not found." << std::endl;
            }
        }

        GLuint getTexture(const std::string& attachmentType) {
            auto it = attachments.find(attachmentType);
            if (it != attachments.end()) {
                return it->second;
            }
            else {
                std::cerr << "ERROR::GBO::getTexture - Attachment type " << attachmentType << " not found." << std::endl;
                return 0;
            }
        }





        void resize(int width, int height) {
            bufferWidth = width;
            bufferHeight = height;
            cleanup(); // Eliminar los recursos existentes
            setupGBuffer(); // Configurar de nuevo el G-Buffer con los nuevos tamaños
            checkGBufferStatus(); // Verificar el estado del framebuffer
        }

        void checkTextures() {
            for (const auto& attachment : attachments) {
                GLuint texture = attachment.second;
                glBindTexture(GL_TEXTURE_2D, texture);

                int width, height, internalFormat;
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

                //std::cout << "Texture: " << attachment.first << " - Width: " << width << ", Height: " << height << ", Internal Format: " << internalFormat << std::endl;

                // Optional: Check texture content for debugging
                std::vector<GLubyte> data(width * height * 4); // Adjust size based on expected channels
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

                // Print first pixel value for debugging
                /*std::cout << "First pixel RGBA of " << attachment.first << ": "
                    << static_cast<int>(data[0]) << ", "
                    << static_cast<int>(data[1]) << ", "
                    << static_cast<int>(data[2]) << ", "
                    << static_cast<int>(data[3]) << std::endl;*/

                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        GLuint gBuffer;

        int bufferWidth, bufferHeight;

    private:
        std::map<std::string, GLuint> attachments;
        

        void setupGBuffer() {
            glGenFramebuffers(1, &gBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

            // Position attachment (world space) view + projection
            GLuint positionTexture = createTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTexture, 0);

            // Position attachment (view space) only view
            GLuint positionViewTexture = createTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, positionViewTexture, 0);

            // Normal attachment
            GLuint normalTexture = createTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);

            // Albedo attachment
            GLuint albedoTexture = createTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedoTexture, 0);

            // Metallic-Roughness attachment
            GLuint metallicRoughnessTexture = createTexture(GL_RGB16F, GL_RGBA, GL_FLOAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, metallicRoughnessTexture, 0);

            // Specular attachment
            GLuint specularTexture = createTexture(GL_R16F, GL_RED, GL_FLOAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, specularTexture, 0);

            // Depth attachment
            GLuint depthTexture = createTexture(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

            // Attachments list
            GLenum attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
            glDrawBuffers(6, attachments);

            this->attachments["position"] = positionTexture;
            this->attachments["positionView"] = positionViewTexture;
            this->attachments["normal"] = normalTexture;
            this->attachments["albedo"] = albedoTexture;
            this->attachments["metallicRoughness"] = metallicRoughnessTexture;
            this->attachments["specular"] = specularTexture;
            this->attachments["depth"] = depthTexture;

            checkGBufferStatus();
            unbindGBuffer();
        }



        GLuint createTexture(GLenum internalFormat, GLenum format, GLenum type) {
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, bufferWidth, bufferHeight, 0, format, type, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            return texture;
        }

        void cleanup() {
            for (auto& attachment : attachments) {
                glDeleteTextures(1, &attachment.second);
            }
            glDeleteFramebuffers(1, &gBuffer);
        }

        void checkGBufferStatus() {
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR::GBufferManager::Framebuffer is not complete!" << std::endl;
            }
            unbindGBuffer();
        }
    };
}