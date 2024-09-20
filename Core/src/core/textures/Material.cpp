#include "Material.h"
#include "../../managers/AssetsManager.h"

namespace libCore
{
    // Constructor predeterminado
    Material::Material()
    {
        albedoMap = AssetsManager::GetInstance().GetTexture("default_albedo");
        normalMap = AssetsManager::GetInstance().GetTexture("default_normal");
        metallicMap = AssetsManager::GetInstance().GetTexture("default_metallic");
        roughnessMap = AssetsManager::GetInstance().GetTexture("default_roughness");
        aOMap = AssetsManager::GetInstance().GetTexture("default_ao");
    }

    // Constructor con nombre de material
    Material::Material(const std::string& matName)
        : materialName(matName)
    {
        albedoMap = AssetsManager::GetInstance().GetTexture("default_albedo");
        normalMap = AssetsManager::GetInstance().GetTexture("default_normal");
        metallicMap = AssetsManager::GetInstance().GetTexture("default_metallic");
        roughnessMap = AssetsManager::GetInstance().GetTexture("default_roughness");
        aOMap = AssetsManager::GetInstance().GetTexture("default_ao");
    }
}


