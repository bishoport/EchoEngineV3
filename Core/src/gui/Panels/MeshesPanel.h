#pragma once
#include "PanelBase.h"
#include "../../managers/ModelLoader.h"  // Ajusta la ruta según tu estructura de proyecto


namespace libCore
{
    class MeshesPanel : public PanelBase
    {
    public:
        MeshesPanel() : PanelBase("Meshes") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            // Obtener las mallas procesadas del ModelLoader
            const auto& meshes = ModelLoader::getInstance().GetProcessedMeshes();

            // Iterar sobre las mallas y mostrar sus datos
            for (const auto& meshEntry : meshes)
            {
                unsigned int meshIndex = meshEntry.first;
                Ref<Mesh> mesh = meshEntry.second;

                ImGui::PushID(meshIndex); // Para evitar conflictos de ID en ImGui

                if (ImGui::CollapsingHeader(mesh->meshName.c_str()))
                {
                    ImGui::Text("Mesh Index: %u", meshIndex);
                    ImGui::Text("Vertex Count: %zu", mesh->vertices.size());
                    ImGui::Text("Index Count: %zu", mesh->indices.size());

                    // Si deseas mostrar más detalles, puedes agregar más información aquí
                    // Por ejemplo, mostrar los primeros vértices
                    if (ImGui::TreeNode("Vertices"))
                    {
                        int count = 0;
                        for (const auto& vertex : mesh->vertices)
                        {
                            ImGui::Text("Vertex %d: Position (%f, %f, %f)", count,
                                vertex.position.x, vertex.position.y, vertex.position.z);
                            count++;
                            if (count >= 5) break; // Mostrar solo los primeros 5 para no saturar
                        }
                        ImGui::TreePop();
                    }
                }

                ImGui::PopID();
                ImGui::Separator();
            }

            //--
            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}
