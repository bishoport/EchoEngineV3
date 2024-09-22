#include "Texture.h"
#include "../../managers/ShaderManager.h"

namespace libCore
{
    // Constructor que recibe un ID de textura
    Texture::Texture(GLuint id, TEXTURE_TYPES type, GLuint slot)
        : ID(id), m_type(type), m_unit(slot)
    {
        // Asigna la textura a una unidad de textura de OpenGL
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    // Vincula la textura al shader
    void Texture::Bind(const std::string& shader)
    {
        if (loadingImage) return;

        // Activa la unidad de textura
        glActiveTexture(GL_TEXTURE0 + m_unit);
        glBindTexture(GL_TEXTURE_2D, ID);

        // Establece el uniform correspondiente en el shader según el tipo de textura
        if (m_type == TEXTURE_TYPES::ALBEDO)
            libCore::ShaderManager::Get(shader)->setInt("albedoTexture", m_unit);
        else if (m_type == TEXTURE_TYPES::NORMAL)
            libCore::ShaderManager::Get(shader)->setInt("normalTexture", m_unit);
        else if (m_type == TEXTURE_TYPES::METALLIC)
            libCore::ShaderManager::Get(shader)->setInt("metallicTexture", m_unit);
        else if (m_type == TEXTURE_TYPES::ROUGHNESS)
            libCore::ShaderManager::Get(shader)->setInt("roughnessTexture", m_unit);
    }

    // Desvincula la textura
    void Texture::Unbind()
    {
        if (loadingImage) return;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Elimina la textura de OpenGL
    void Texture::Delete()
    {
        if (loadingImage) return;
        glDeleteTextures(1, &ID);
    }

    // Obtiene el ID de la textura
    GLuint Texture::GetTextureID() const
    {
        if (loadingImage) return 0;
        return ID;
    }

    // Obtiene el tipo de la textura
    TEXTURE_TYPES Texture::GetType() const
    {
        return m_type;
    }
}
