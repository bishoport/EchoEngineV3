#pragma once

#include "../Core.h"

namespace libCore
{
    class Texture
    {
    public:
        GLuint ID = 0;                     // ID de la textura en OpenGL
        TEXTURE_TYPES m_type = TEXTURE_TYPES::ALBEDO;  // Tipo de textura (Albedo, Normal, etc.)
        GLuint m_unit = 0;                 // Unidad de textura en OpenGL
        std::string m_textureName = "";    // Nombre de la textura
        std::string texturePath = "";      // Ruta de la textura en el disco

        std::string key = "";

        bool loadingImage = false;

        // Constructor predeterminado
        Texture() = default;

        // Constructor que recibe un ID de textura ya cargado
        Texture(GLuint id, TEXTURE_TYPES type, GLuint slot);

        // Vincula la textura al shader
        void Bind(const std::string& shader);

        // Desvincula la textura
        void Unbind();

        // Elimina la textura de OpenGL
        void Delete();

        // Verifica si la textura es válida
        bool IsValid() const { return ID != 0; }

        // Obtiene el ID de la textura
        GLuint GetTextureID() const;

        // Obtiene el tipo de la textura
        TEXTURE_TYPES GetType() const;
    };
}
