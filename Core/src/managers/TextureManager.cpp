#include "TextureManager.h"
#include "EventManager.h"

#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>



namespace fs = std::filesystem;

namespace libCore
{

    Ref<Texture> TextureManager::LoadTexture(const char* directoryPath, const char* fileName, TEXTURE_TYPES type, GLuint slot)
    {
        // Función auxiliar para buscar la textura recursivamente en subcarpetas
        auto findTextureInSubfolders = [](const fs::path& baseDir, const std::string& imageName) -> fs::path
        {
            for (auto& p : fs::recursive_directory_iterator(baseDir))
            {
                if (fs::is_regular_file(p) && p.path().filename() == imageName)
                {
                    return p.path();
                }
            }
            return "";
        };

        // Ruta completa de la textura
        fs::path imagePathFS = fs::path(directoryPath) / fileName;
        std::string textureKey = imagePathFS.string();

        // Verificar si la textura ya está cargada en caché
        auto it = loadedTextures.find(textureKey);
        if (it != loadedTextures.end()) {
            ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Textura encontrada en memoria: " + textureKey);
            return it->second;  // Retorna la textura si ya está cargada
        }

        // Comprobar si la textura existe en la ruta inicial
        if (!fs::exists(imagePathFS))
        {
            // Registrar que no se encontró la textura en la ruta inicial
            ConsoleLog::GetInstance().AddLog(LogLevel::L_WARNING, "Textura no encontrada en la ruta inicial. Buscando en subcarpetas...");

            // Buscar recursivamente en las subcarpetas
            fs::path foundPath = findTextureInSubfolders(directoryPath, fileName);

            if (foundPath.empty())
            {
                ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Textura no encontrada en subcarpetas: " + std::string(fileName));
                return nullptr;
            }

            imagePathFS = foundPath;  // Actualizar la ruta de la textura encontrada
            textureKey = imagePathFS.string();
            ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Textura encontrada en: " + textureKey);
        }

        // Cargar la imagen con stb_image
        int width, height, nrChannels;
        unsigned char* data = stbi_load(imagePathFS.string().c_str(), &width, &height, &nrChannels, 0);
        if (!data) {
            ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Error al cargar la textura: " + textureKey);
            return nullptr;
        }

        // Generar la textura en OpenGL
        GLuint textureID = GenerateTexture(data, width, height, nrChannels, type, slot);

        // Liberar la memoria de la imagen cargada
        stbi_image_free(data);

        // Crear una nueva instancia de la textura y guardarla en el mapa de texturas cargadas
        auto texture = CreateRef<Texture>(textureID, type, slot);
        loadedTextures[textureKey] = texture;

        ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Textura cargada correctamente: " + textureKey);

        return texture;
    }


    std::future<Ref<Texture>> TextureManager::LoadTextureAsync(const char* directoryPath, const char* fileName, TEXTURE_TYPES type, GLuint slot, std::atomic<float>& progress)
    {
        std::promise<Ref<Texture>> promise;
        std::future<Ref<Texture>> future = promise.get_future();

        std::thread([this, directoryPath, fileName, type, slot, &progress, promise = std::move(promise)]() mutable {
            try {
                // Emitir evento de inicio
                EventManager::OnLoadAssetStart().trigger(fileName);

                // Simulación de progreso de carga inicial
                progress = 0.1f;
                EventManager::OnLoadAssetProgress().trigger(progress);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Simular tiempo de carga

                // Cargar la textura
                Ref<Texture> texture = LoadTexture(directoryPath, fileName, type, slot);

                if (texture) {
                    progress = 1.0f;
                    EventManager::OnLoadAssetProgress().trigger(progress);  // Emitir progreso final
                    EventManager::OnLoadAssetComplete().trigger(fileName, true);  // Carga exitosa
                    promise.set_value(texture);
                    texture->loadingImage = false;
                }
                else {
                    throw std::runtime_error("Failed to load texture.");
                }

            }
            catch (const std::exception& e) {
                EventManager::OnLoadAssetFailed().trigger(fileName, e.what());  // Error en la carga
                promise.set_value(nullptr);
            }
            }).detach();

            return future;
    }




    // Función auxiliar que genera una textura de OpenGL a partir de los datos de imagen cargados
    GLuint TextureManager::GenerateTexture(unsigned char* data, int width, int height, int nrChannels, TEXTURE_TYPES type, GLuint slot)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Determina el formato de la textura basado en el número de canales
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        // Carga los datos de la textura en OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Establece los parámetros de la textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        return textureID;
    }
    



    GLuint TextureManager::LoadImagesForCubemap(std::vector<const char*> faces)
    {
        stbi_set_flip_vertically_on_load(false);

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrComponents;


        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char* data = stbi_load(faces[i], &width, &height, &nrComponents, 0);

            if (data)
            {
                ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Cubemap Texture Loaded:" + std::string(faces[i]));
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Cubemap texture failed to load at path:" + std::string(faces[i]));
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    GLuint libCore::TextureManager::loadHDR(const char* filepath)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        float* data = stbi_loadf(filepath, &width, &height, &nrComponents, 0);

        unsigned int hdrTexture{};

        if (data)
        {
            glGenTextures(1, &hdrTexture);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            ConsoleLog::GetInstance().AddLog(LogLevel::L_SUCCESS, "Image HDR loaded successfully");
        }
        else
        {
            ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Failed to load HDR image");
        }

        return hdrTexture;
    }

    void libCore::TextureManager::SaveImage(const std::string& filePath, int width, int height, int channels, unsigned char* data)
    {
        stbi_flip_vertically_on_write(false); // Voltear verticalmente la imagen para coincidir con las coordenadas de OpenGL
        stbi_write_png(filePath.c_str(), width, height, channels, data, width * channels);
    }


    // Carga una textura desde un archivo y la devuelve como GLuint
    GLuint TextureManager::LoadTextureFromFile(const char* filePath)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);

        if (data)
        {
            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            return textureID;
        }
        else
        {
            ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Failed to load Texture FromFile: " + std::string(filePath));
            return 0;
        }
    }
}
