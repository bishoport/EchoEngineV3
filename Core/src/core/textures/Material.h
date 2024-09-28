#pragma once

#include "../Core.h"
#include "Texture.h"

namespace libCore
{
    class Material
    {
    public:
        // Constructor predeterminado
        Material();

        // Constructor con nombre de material
        Material(const std::string& matName);

        std::string materialName = "default_material";
        
        //std::string shaderName = "default";

        // Valores
        glm::vec3 albedoColor = glm::vec3(1.0f, 1.0f, 1.0f);
        float normalStrength = 1.0f;
        float metallicValue = 0.0f;
        float roughnessValue = 0.7f;
        float aoValue = 1.0f;

        // Texturas
        Ref<Texture> albedoMap;
        Ref<Texture> normalMap;
        Ref<Texture> metallicMap;
        Ref<Texture> roughnessMap;
        Ref<Texture> aOMap;
    };
}
