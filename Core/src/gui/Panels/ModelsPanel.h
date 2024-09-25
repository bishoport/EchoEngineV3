#pragma once
#include "PanelBase.h"
#include "../../managers/AssetsManager.h"
#include "../../managers/TextureLoader.h"
#include "../../managers/EntityManager.h"

namespace libCore
{
    class ModelsPanel : public PanelBase
    {
    public:
        ModelsPanel() : PanelBase("Models"), m_previewSize(64.0f) {}

        void Init() override
        {
        }

        // Función para dibujar la jerarquía del modelo
        void DrawModelHierarchy(const Ref<libCore::Model>& model)
        {
            if (!model) return;

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);

            // Crear un nodo del árbol para el modelo
            if (ImGui::TreeNode((ICON_FA_CUBES + model->name).c_str()))  // Añadir icono de modelo
            {
                // Mostrar información sobre el número de huesos si el modelo tiene datos esqueléticos
                if (!model->m_BoneInfoMap.empty())
                {
                    ImGui::Text("Huesos en el modelo: %d", model->GetBoneCount());

                    if (ImGui::TreeNode("Huesos del modelo"))
                    {
                        for (const auto& bone : model->GetBoneInfoMap())
                        {
                            ImGui::Text("Hueso: %s, ID: %d", bone.first.c_str(), bone.second.id);
                            ImGui::Text("Offset: [%.2f, %.2f, %.2f]", bone.second.offset[3][0], bone.second.offset[3][1], bone.second.offset[3][2]);
                        }
                        ImGui::TreePop();
                    }
                }
                else
                {
                    ImGui::Text("No hay huesos asociados a este modelo.");
                }

                // Recorrer los meshes del modelo
                for (const auto& mesh : model->meshes)
                {
                    DrawMeshWithThumbnail(mesh, model->importModelData.filePath);

                    if (ImGui::Button((ICON_FA_PLUS " " + model->name).c_str()))  // Añadir icono de instanciación
                    {
                        // Lógica de instanciación del modelo
                        EntityManager::GetInstance().CreateGameObjectFromModel(model, entt::null);
                    }

                    // Iniciar fuente de drag and drop para los modelos
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        ImGui::SetDragDropPayload("MODEL_PAYLOAD", model->name.c_str(), model->name.size() + 1);
                        ImGui::Text((std::string(ICON_FA_HAND_ROCK) + " Dragging: " + model->name).c_str());
                        ImGui::EndDragDropSource();
                    }
                }

                // Recorrer recursivamente los hijos del modelo y mostrar en un grid
                DrawModelChildrenInGrid(model->children);

                ImGui::TreePop();
            }
        }


        // Función para mostrar los hijos del modelo en un grid ajustable
        void DrawModelChildrenInGrid(const std::vector<Ref<libCore::Model>>& children)
        {
            // Determinar el tamaño del panel
            ImVec2 panelSize = ImGui::GetContentRegionAvail();
            int columns = std::max(1, static_cast<int>(panelSize.x / (m_previewSize + 10))); // Espaciado entre imágenes
            if (ImGui::BeginTable("ModelChildrenTable", columns))
            {
                for (const auto& child : children)
                {
                    ImGui::TableNextColumn();
                    DrawMeshWithThumbnail(child->meshes[0], child->modelParent->importModelData.filePath); // Mostrar solo el primer mesh

                    // Añadir botón de instanciación para cada hijo
                    if (ImGui::Button((ICON_FA_PLUS " " + child->name).c_str()))  // Añadir icono de instanciación para hijos
                    {
                        EntityManager::GetInstance().CreateGameObjectFromModel(child, entt::null);
                    }

                    // Permitir drag and drop para cada modelo hijo
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        ImGui::SetDragDropPayload("MODEL_PAYLOAD", child->name.c_str(), child->name.size() + 1);
                        ImGui::Text((std::string(ICON_FA_HAND_ROCK) + " Dragging: " + child->name).c_str());
                        ImGui::EndDragDropSource();
                    }
                }
                ImGui::EndTable();
            }
        }

        // Función para mostrar el thumbnail del mesh
        void DrawMeshWithThumbnail(const Ref<Mesh>& mesh, const std::string& modelPath)
        {
            if (mesh->thumbnailTextureID == 0)
            {
                // Ruta y carga del thumbnail
                std::filesystem::path projectBasePath = std::filesystem::current_path();
                std::filesystem::path thumbnailPath = projectBasePath / modelPath / "thumbnails" / (mesh->meshName + "_thumbnail.png");

                // Verificar si el archivo existe antes de intentar cargarlo
                if (std::filesystem::exists(thumbnailPath)) {
                    mesh->thumbnailTextureID = TextureLoader::getInstance().LoadTextureFromFile(thumbnailPath.string().c_str());
                }
                else {
                    std::cerr << "Error: No se encontró el archivo de thumbnail: " << thumbnailPath.string() << std::endl;
                }
            }


            ImVec2 imageSize(m_previewSize, m_previewSize);

            if (mesh->thumbnailTextureID != 0)
            {
                ImGui::Image((void*)(intptr_t)mesh->thumbnailTextureID, imageSize);
            }
            else
            {
                ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE " No thumbnail");  // Añadir icono de advertencia si no hay thumbnail
            }
        }

        void Draw() override {
            if (!m_isVisible) return;

            auto& assetsManager = libCore::AssetsManager::GetInstance();
            const auto& models = assetsManager.GetAllModels();

            ImGui::Begin(m_title.c_str());
            //-- 

            // Slider para ajustar el tamaño de las previsualizaciones
            ImGui::SliderFloat(ICON_FA_EXPAND_ARROWS_ALT " Preview Size", &m_previewSize, 64.0f, 256.0f, "%.0f");  // Añadir icono de slider

            for (const auto& modelPair : models)
            {
                const auto& model = modelPair.second;

                if (model)
                {
                    ImGui::Separator();
                    ImGui::Text(ICON_FA_CUBE " Model: %s", model->name.c_str());  // Añadir icono de modelo

                    if (ImGui::Button((ICON_FA_PLUS + model->name).c_str()))  // Añadir icono de instanciación
                    {
                        // Lógica de instanciación del modelo
                        EntityManager::GetInstance().CreateGameObjectFromModel(model, entt::null);
                    }

                    // Iniciar fuente de drag and drop para los modelos
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        ImGui::SetDragDropPayload("MODEL_PAYLOAD", model->name.c_str(), model->name.size() + 1);
                        ImGui::Text((std::string(ICON_FA_HAND_ROCK) + " Dragging: " + model->name).c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Dibujar la jerarquía del modelo
                    DrawModelHierarchy(model);
                }
                else
                {
                    ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE " Invalid model");  // Añadir icono de advertencia para modelos inválidos
                }
            }

            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }

    private:
        float m_previewSize;  // Tamaño ajustable de las previsualizaciones
    };
}


//#pragma once
//#include "PanelBase.h"
//#include "../../managers/AssetsManager.h"
//#include "../../managers/TextureManager.h"
//#include "../../managers/EntityManager.h"
//
//namespace libCore
//{
//    class ModelsPanel : public PanelBase
//    {
//    public:
//        ModelsPanel() : PanelBase("Models") {}
//
//        void Init() override
//        {
//
//        }
//
//        void DrawModelHierarchy(const Ref<libCore::Model>& model)
//        {
//            if (!model) return;
//
//            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//
//            // Crear un nodo del árbol para el modelo
//            if (ImGui::TreeNode(model->name.c_str()))
//            {
//                // Recorrer los meshes del modelo
//                for (const auto& mesh : model->meshes)
//                {
//                    DrawMeshWithThumbnail(mesh, model->modelParent->importModelData.filePath);
//
//                    if (ImGui::Button(("Instantiate " + model->name).c_str()))
//                    {
//                        // Lógica de instanciación del modelo
//                        EntityManager::GetInstance().CreateGameObjectFromModel(model, entt::null);
//                    }
//                    // Iniciar fuente de drag and drop para los modelos
//                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
//                    {
//                        // Enviar el nombre del modelo como payload
//                        ImGui::SetDragDropPayload("MODEL_PAYLOAD", model->name.c_str(), model->name.size() + 1);
//                        ImGui::Text("Dragging: %s", model->name.c_str());
//                        ImGui::EndDragDropSource();
//                    }
//                }
//
//                // Recorrer recursivamente los hijos del modelo
//                for (const auto& child : model->children)
//                {
//                    DrawModelHierarchy(child);
//                }
//
//                ImGui::TreePop();
//            }
//        }
//        // Muestra el thumbnail junto con el nombre del mesh
//        void DrawMeshWithThumbnail(const Ref<Mesh>& mesh, const std::string& modelPath)
//        {
//            if (mesh->thumbnailTextureID == 0)
//            {
//                // Obtener la ruta absoluta del directorio de trabajo actual
//                std::filesystem::path projectBasePath = std::filesystem::current_path();
//
//                std::string assetsPath = "assets/models";
//
//                // Construir la ruta completa para el thumbnail
//                std::filesystem::path thumbnailPath = projectBasePath / modelPath / "thumbnails" / (mesh->meshName + "_thumbnail.png");
//
//                // Convertir la ruta a string y cargar la textura
//                mesh->thumbnailTextureID = TextureManager::getInstance().LoadTextureFromFile(thumbnailPath.string().c_str());
//            }
//
//
//            ImVec2 imageSize(128, 128);
//
//            if (mesh->thumbnailTextureID != 0)
//            {
//                ImGui::Image((void*)(intptr_t)mesh->thumbnailTextureID, imageSize);
//            }
//            else
//            {
//                ImGui::Text("No thumbnail");
//            }
//
//            ImGui::SameLine();
//            ImGui::Text("Mesh: %s", mesh->meshName.c_str());
//        }
//
//        void Draw() override {
//            if (!m_isVisible) return;
//
//            auto& assetsManager = libCore::AssetsManager::GetInstance();
//            const auto& models = assetsManager.GetAllModels();
//
//            ImGui::Begin(m_title.c_str());
//            //--
//
//            ImGui::Spacing();
//            ImGui::Text("Model Count: %zu", models.size());
//            ImGui::Spacing();
//
//            for (const auto& modelPair : models)
//            {
//                const auto& model = modelPair.second;
//
//                if (model)
//                {
//                    ImGui::Separator();
//                    ImGui::Text("Model: %s", model->name.c_str());
//
//                    if (ImGui::Button(("Instantiate " + model->name).c_str()))
//                    {
//                        // Lógica de instanciación del modelo
//                        EntityManager::GetInstance().CreateGameObjectFromModel(model, entt::null);
//                    }
//
//
//                    // Iniciar fuente de drag and drop para los modelos
//                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
//                    {
//                        // Enviar el nombre del modelo como payload
//                        ImGui::SetDragDropPayload("MODEL_PAYLOAD", model->name.c_str(), model->name.size() + 1);
//                        ImGui::Text("Dragging: %s", model->name.c_str());
//                        ImGui::EndDragDropSource();
//                    }
//
//                    // Dibujar la jerarquía del modelo (si corresponde)
//                    DrawModelHierarchy(model);
//                }
//                else
//                {
//                    ImGui::Text("Invalid model");
//                }
//            }
//
//            //--
//            ImGui::End();
//        }
//
//        void Shutdown() override {
//            // Liberación de recursos si es necesario
//        }
//    };
//}
