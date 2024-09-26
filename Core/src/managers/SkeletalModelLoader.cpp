#include "SkeletalModelLoader.h"

#include "../managers/TextureLoader.h"
#include "../managers/LightsManager.hpp"
#include "../managers/AssetsManager.h"

#include "../core/textures/Material.h"
#include "../core/lights/Light.h"

#include "../core/model/skeletal/animation.h"

#include "../tools/AssimpGlmHelpers.h"

namespace libCore
{
    namespace fs = std::filesystem;

    Ref<Model> SkeletalModelLoader::LoadModel(ImportModelData importOptions)
    {
        current_importOptions = importOptions;


        auto modelParent = CreateRef<Model>();
        modelParent->importModelData = importOptions;

        Assimp::Importer importer;
        std::string completePath = importOptions.filePath + importOptions.fileName;

        unsigned int flags = aiProcess_Triangulate;
        flags |= aiProcess_CalcTangentSpace;
        flags |= aiProcess_GenSmoothNormals;

        if (importOptions.invertUV == true) flags |= aiProcess_FlipUVs;

        const aiScene* scene = importer.ReadFile(completePath, flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return nullptr;  //--> lo deolvemos vacio
        }



        aiMatrix4x4 nodeTransform = scene->mRootNode->mTransformation;
        modelParent->name = importOptions.fileName;

        processNode(scene->mRootNode, scene, modelParent, nodeTransform);

        return modelParent;
    }


    void SkeletalModelLoader::processNode(aiNode* node, const aiScene* scene, Ref<Model> modelParent, aiMatrix4x4 _nodeTransform)
    {
        glm::mat4 glmNodeTransform = aiMatrix4x4ToGlm(_nodeTransform);

        // Procesar las mallas en el nodo actual
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {

            unsigned int meshIndex = node->mMeshes[i];
            aiMesh* mesh = scene->mMeshes[meshIndex];

            //auto modelChild = CreateRef<Model>();
            //modelChild->importModelData = current_importOptions;
            //// Aquí establecemos la relación padre-hijo
            //modelChild->modelParent = modelParent;

            //// Asignar el nombre del nodo de Assimp al modelo
            //modelChild->name = node->mName.C_Str();

            //// Establecer la posición del modelo basado en la transformación final
            //modelChild->transform->position = glm::vec3(glmNodeTransform[3]);

            // Procesar la malla y almacenarla en el mapa
            processMesh(mesh, scene, modelParent, _nodeTransform, meshIndex);

            processMaterials(mesh, scene, modelParent);

            //modelParent->children.push_back(modelChild);
        }

        // Procesar los nodos hijos recursivamente
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, modelParent, _nodeTransform);
        }

    }

    void SkeletalModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild, aiMatrix4x4 finalTransform, int meshIndex)
    {

        auto meshBuild = CreateRef<Mesh>();

        // Cargando los datos de los vértices y los índices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
            vertex.normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);


            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texUV = vec;
            }
            else
                vertex.texUV = glm::vec2(0.0f, 0.0f);

            
            vertex.normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
            vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);
            vertex.bitangent = glm::vec3(0.0f, 0.0f, 0.0f);

            meshBuild->vertices.push_back(vertex);
            //--------------------------------------------------------------
        }

        //-INDICES
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                meshBuild->indices.push_back(face.mIndices[j]);
            }
        }
        meshBuild->drawLike = DRAW_GEOM_LIKE::TRIANGLE;

        ExtractBoneWeightForVertices(meshBuild, modelBuild, mesh, scene);

        //-MESH ID
        std::string meshNameBase = mesh->mName.C_Str();
        meshBuild->meshName = meshNameBase;
        meshBuild->SetupMesh();
        modelBuild->meshes.push_back(meshBuild);
    }
    void SkeletalModelLoader::ExtractBoneWeightForVertices(Ref<Mesh> meshBuild, Ref<Model> modelBuild, aiMesh* mesh, const aiScene* scene)
    {
        auto& boneInfoMap = modelBuild->m_BoneInfoMap;
        int& boneCount = modelBuild->m_BoneCounter;

        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;
            }
            else
            {
                boneID = boneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= meshBuild->vertices.size());
                SetVertexBoneData(meshBuild->vertices[vertexId], boneID, weight);
            }
        }

        // Confirmar que se ha extraído información de huesos
        if (boneInfoMap.empty()) {
            std::cerr << "No se ha procesado ningún hueso." << std::endl;
        }
        else {
            std::cout << "Total huesos procesados: " << boneCount << std::endl;
        }
    }


    void SkeletalModelLoader::SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }
    void SkeletalModelLoader::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }
    
    
    
    void SkeletalModelLoader::processMaterials(aiMesh* mesh, const aiScene* scene, Ref<Model> modelBuild)
    {
        // Obtener el material del mesh
        const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        std::string materialName = mat->GetName().C_Str();

        // Verificar si el material ya existe en el MaterialManager
        auto existingMaterial = AssetsManager::GetInstance().getMaterial(materialName);

        Ref<Material> material;

        if (existingMaterial)
        {
            material = existingMaterial;
        }
        else
        {
            // Crear un nuevo material
            material = CreateRef<Material>(materialName);

            // Obtener el color difuso del material
            aiColor3D color(1.f, 1.f, 1.f);
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material->albedoColor = { color.r, color.g, color.b };

            aiString texturePath;

            //--ALBEDO
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                std::string completePathTexture = texturePath.C_Str();
                // std::cout << "Loading Texture: " << completePathTexture << std::endl;

                std::string directoryPath = fs::path(current_importOptions.filePath).parent_path().string();
                std::string fileName = fs::path(completePathTexture).filename().string();

                std::string key = materialName + "_ALBEDO";

                Ref<Texture> texture = AssetsManager::GetInstance().LoadTextureAsset(key, directoryPath.c_str(), fileName.c_str(), TEXTURE_TYPES::ALBEDO);

                if (texture != nullptr) {
                    material->albedoMap = texture;
                }
                else {
                    material->albedoMap = AssetsManager::GetInstance().GetTexture("default_albedo");
                }
            }
            else {
                material->albedoMap = AssetsManager::GetInstance().GetTexture("default_albedo");
            }

            //--NORMAL
            if (mat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
                std::string completePathTexture = texturePath.C_Str();
                //std::cout << "Loading Normal Map: " << completePathTexture << std::endl;

                std::string directoryPath = fs::path(current_importOptions.filePath).parent_path().string();
                std::string fileName = fs::path(completePathTexture).filename().string();

                std::string key = materialName + "_NORMAL";
                Ref<Texture> texture = AssetsManager::GetInstance().LoadTextureAsset(key, directoryPath.c_str(), fileName.c_str(), TEXTURE_TYPES::NORMAL);
                //Ref<Texture> texture = libCore::TextureManager::LoadTexture(directoryPath.c_str(), fileName.c_str(), TEXTURE_TYPES::NORMAL, 1);

                if (texture != nullptr)
                {
                    material->normalMap = texture;
                }
                else
                {
                    material->normalMap = AssetsManager::GetInstance().GetTexture("default_normal");
                }
            }
            else {
                material->normalMap = AssetsManager::GetInstance().GetTexture("default_normal");
            }

            //--METALLIC
            if (mat->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS) {
                std::string completePathTexture = texturePath.C_Str();
                // std::cout << "Loading Metallic Map: " << completePathTexture << std::endl;

                std::string directoryPath = fs::path(current_importOptions.filePath).parent_path().string();
                std::string fileName = fs::path(completePathTexture).filename().string();

                std::string key = materialName + "_METALLIC";
                Ref<Texture> texture = AssetsManager::GetInstance().LoadTextureAsset(key, directoryPath.c_str(), fileName.c_str(), TEXTURE_TYPES::METALLIC);

                //Ref<Texture> texture = libCore::TextureManager::LoadTexture(directoryPath.c_str(), fileName.c_str(), TEXTURE_TYPES::METALLIC, 2);

                if (texture != nullptr) {
                    material->metallicMap = texture;
                }
                else {
                    material->metallicMap = AssetsManager::GetInstance().GetTexture("default_metallic");
                }
            }
            else {
                material->metallicMap = AssetsManager::GetInstance().GetTexture("default_metallic");
            }

            //--ROUGHNESS
            if (mat->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS) {
                std::string completePathTexture = texturePath.C_Str();
                //std::cout << "Loading Roughness Map: " << completePathTexture << std::endl;

                std::string directoryPath = fs::path(current_importOptions.filePath).parent_path().string();
                std::string fileName = fs::path(completePathTexture).filename().string();

                std::string key = materialName + "_ROUGHNESS";
                Ref<Texture> texture = AssetsManager::GetInstance().LoadTextureAsset(key, directoryPath.c_str(), fileName.c_str(), TEXTURE_TYPES::ROUGHNESS);

                if (texture != nullptr) {
                    material->roughnessMap = texture;
                }
                else {
                    material->roughnessMap = AssetsManager::GetInstance().GetTexture("default_roughness");
                }
            }
            else {
                material->roughnessMap = AssetsManager::GetInstance().GetTexture("default_roughness");
            }

            // Añadir el material al MaterialManager
            AssetsManager::GetInstance().addMaterial(material);
        }

        // Añadir el material al modelo
        modelBuild->materials.push_back(material);
    }

    //-------------------------------------TOOLS---------------------------
    glm::mat4 SkeletalModelLoader::aiMatrix4x4ToGlm(const aiMatrix4x4& from)
    {
        glm::mat4 to;

        // Transponer y convertir a glm
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;

        return to;
    }
    aiMatrix4x4 SkeletalModelLoader::glmToAiMatrix4x4(const glm::mat4& from)
    {
        aiMatrix4x4 to;

        // Transponer y convertir a Assimp
        to.a1 = from[0][0]; to.a2 = from[1][0]; to.a3 = from[2][0]; to.a4 = from[3][0];
        to.b1 = from[0][1]; to.b2 = from[1][1]; to.b3 = from[2][1]; to.b4 = from[3][1];
        to.c1 = from[0][2]; to.c2 = from[1][2]; to.c3 = from[2][2]; to.c4 = from[3][2];
        to.d1 = from[0][3]; to.d2 = from[1][3]; to.d3 = from[2][3]; to.d4 = from[3][3];

        return to;
    }
    std::string SkeletalModelLoader::getFileName(const std::string& path) {
        size_t lastSlash = path.find_last_of("\\/");
        if (lastSlash == std::string::npos) {
            return path; // No hay ningún separador, toda la cadena es el nombre del archivo
        }
        return path.substr(lastSlash + 1);
    }
}

