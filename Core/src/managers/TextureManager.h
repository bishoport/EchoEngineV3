#pragma once
#include "../core/Core.h"
#include "../core/textures/Texture.h"

#include <future> // Para std::future y std::promise
#include <atomic> // Para manejar el progreso de forma segura entre hilos

namespace libCore
{
    class TextureManager
    {
    public:
        static TextureManager& getInstance() {
            static TextureManager instance;
            return instance;
        }

        std::future<Ref<Texture>> LoadTextureAsync(const char* directoryPath, const char* fileName, TEXTURE_TYPES type, GLuint slot, std::atomic<float>& progress);

        Ref<Texture> LoadTexture(const char* directoryPath, const char* fileName, TEXTURE_TYPES type, GLuint slot);

        // Existing methods for cubemaps and HDR textures
        GLuint LoadImagesForCubemap(std::vector<const char*> faces);
        GLuint loadHDR(const char* filepath);
        void SaveImage(const std::string& filePath, int width, int height, int channels, unsigned char* data);
        GLuint LoadTextureFromFile(const char* filePath);

    private:
        std::unordered_map<std::string, Ref<Texture>> loadedTextures;
        TextureManager() {}

        GLuint GenerateTexture(unsigned char* data, int width, int height, int nrChannels, TEXTURE_TYPES type, GLuint slot);
    };
}