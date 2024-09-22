#pragma once
#include "../core/Core.h"
#include "../core/textures/Texture.h"


namespace libCore
{
    class TextureLoader
    {
    public:
        static TextureLoader& getInstance() {
            static TextureLoader instance;
            return instance;
        }

        Ref<Texture> LoadTexture(const char* directoryPath, const char* fileName, TEXTURE_TYPES type, GLuint slot);

        
        

    private:
        TextureLoader() {}
        GLuint GenerateTexture(unsigned char* data, int width, int height, int nrChannels, TEXTURE_TYPES type, GLuint slot);
        
    public:
        // Existing methods for cubemaps and HDR textures
        GLuint LoadImagesForCubemap(std::vector<const char*> faces);
        GLuint loadHDR(const char* filepath);
        void SaveImage(const std::string& filePath, int width, int height, int channels, unsigned char* data);
        GLuint LoadTextureFromFile(const char* filePath);

        
    };
}