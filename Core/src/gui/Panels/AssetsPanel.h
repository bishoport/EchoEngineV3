#pragma once
#include "PanelBase.h"
#include "../../managers/AssetsManager.h"
//#include "../../managers/SceneManager.h"

namespace libCore
{
    class AssetsPanel : public PanelBase
    {
    public:
        AssetsPanel()
            : PanelBase("Assets"), m_CurrentDirectory("assets"), m_BaseDirectory("assets")
        {
            // Inicialización de íconos (suponiendo que son texturas de OpenGL)
            iconFolder = 0;
            iconModel = 0;
            iconImage = 0;
        }

        void Init() override
        {
            m_CurrentDirectory = m_BaseDirectory;
        }

        void Draw() override
        {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //-- 
            ImGui::BeginChild("FileRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

            if (m_CurrentDirectory != std::filesystem::path("assets")) {
                if (ImGui::Button(ICON_FA_ARROW_LEFT " Back")) {  // Icono para volver atrás
                    m_CurrentDirectory = m_CurrentDirectory.parent_path();
                }
            }

            static float padding = 0.0f;
            static float thumbnailSize = 64.0f;
            float cellSize = thumbnailSize + padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columnCount = thumbnailSize < 40 ? 1 : (int)(panelWidth / cellSize);

            if (columnCount < 1)
                columnCount = 1;

            ImGui::Columns(columnCount, 0, false);

            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
                const auto& path = directoryEntry.path();
                auto relativePath = std::filesystem::relative(path, "assets");
                std::string filenameString = path.filename().string();

                ImGui::PushID(filenameString.c_str());

                GLuint iconTexture = 0;
                const char* iconLabel = ICON_FA_FILE;  // Icono por defecto para archivos no reconocidos

                //-- Asignar los iconos apropiados basados en la extensión o si es una carpeta
                if (directoryEntry.is_directory()) {
                    iconLabel = ICON_FA_FOLDER;  // Icono de carpeta
                }
                else if (path.extension() == ".fbx" || path.extension() == ".obj" || path.extension() == ".gltf") {
                    iconLabel = ICON_FA_CUBE;  // Icono de modelos 3D
                }
                else if (path.extension() == ".png" || path.extension() == ".jpg") {
                    iconLabel = ICON_FA_IMAGE;  // Icono para imágenes
                }
                else if (path.extension() == ".lua") {
                    iconLabel = ICON_FA_CODE;  // Icono para archivos Lua
                }

                // Establecer el tamaño de los iconos de manera uniforme
                ImVec2 iconSize(thumbnailSize, thumbnailSize);

                // Usar botones con los iconos correspondientes
                if (ImGui::Button(iconLabel, iconSize)) {
                    // Acción al hacer clic en el botón, por ejemplo, abrir la carpeta o archivo
                    if (directoryEntry.is_directory()) {
                        m_CurrentDirectory /= path.filename();
                    }
                    else if (path.extension() == ".lua") {
                        std::string scriptName = path.stem().string();
                        LuaManager::GetInstance().LoadLuaFile(scriptName, path.string());
                    }
                    else if (path.extension() == ".fbx" || path.extension() == ".obj" || path.extension() == ".gltf") {
                        isDialogOpen = true;  // Abre el diálogo
                        m_DirectoryEntry = directoryEntry;
                    }
                }

                // Funcionalidad de arrastrar y soltar
                if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0, 0.0f)) {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                        const std::string payload_n = relativePath.string();
                        ImGui::SetDragDropPayload("ASSET_DRAG", payload_n.c_str(), payload_n.length() + 1, ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }
                }

                ImGui::TextWrapped(filenameString.c_str());  // Mostrar el nombre del archivo/carpeta
                ImGui::NextColumn();

                ImGui::PopID();
            }

            if (isDialogOpen) {
                ImGui::OpenPopup("Import Options");
            }

            if (ImGui::BeginPopupModal("Import Options", &isDialogOpen)) {
                std::filesystem::path fullPath = m_DirectoryEntry;
                std::string filePath = fullPath.parent_path().string();
                std::string fileName = fullPath.filename().string();

                ImGui::Text("Path: %s", filePath.c_str());
                ImGui::Text("File: %s", fileName.c_str());
                ImGui::Separator();

                ImGui::Checkbox("Invert UV", &importOptions.invertUV);
                ImGui::Checkbox("Rotate 90 degrees on X-axis", &importOptions.rotate90);
                ImGui::SliderFloat("Scale", &importOptions.globalScaleFactor, 0.01f, 10.0f, "%.1f");
                ImGui::Separator();

                ImGui::Checkbox("Lights", &importOptions.processLights);

                // Variables estáticas para manejar la carga
                static std::atomic<float> loadProgress = 0.0f;
                static bool isLoading = false;
                static bool loadComplete = false;
                static bool loadFailed = false;
                static std::string loadError;

                if (!isLoading) {
                    if (ImGui::Button("Accept")) {
                        isDialogOpen = false;

                        if (filePath.back() != '\\' && filePath.back() != '/') {
                            filePath += '\\';
                        }
                        importOptions.filePath = filePath;
                        importOptions.fileName = fileName;

                        // Iniciar la carga en un hilo separado
                        isLoading = true;
                        loadComplete = false;
                        loadFailed = false;
                        loadProgress = 0.0f;

                        // Suscribirse a eventos de progreso y finalización
                        EventManager::OnLoadAssetStart().subscribe([](const std::string& fileName) {
                            // Código opcional para manejar el inicio
                            std::cout << "Started loading: " << fileName << std::endl;
                            });

                        EventManager::OnLoadAssetProgress().subscribe([](float progress) {
                            loadProgress = progress;
                            });

                        EventManager::OnLoadAssetComplete().subscribe([](const std::string& fileName, bool success) {
                            if (success) {
                                loadComplete = true;
                                isLoading = false;
                            }
                            });

                        EventManager::OnLoadAssetFailed().subscribe([](const std::string& fileName, const std::string& error) {
                            loadFailed = true;
                            loadError = error;
                            isLoading = false;
                            });

                        // Llamar a la carga asíncrona
                        //AssetsManager::GetInstance().LoadModelAssetAsync(importOptions);
                        AssetsManager::GetInstance().LoadModelAsset(importOptions);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        isDialogOpen = false;
                    }
                }
                else {
                    // Mostrar el progreso de carga
                    ImGui::Text("Loading...");
                    ImGui::ProgressBar(loadProgress);
                    ImGui::Dummy(ImVec2(0.0f, 5.0f));

                    // Si la carga está completa o ha fallado
                    if (loadComplete) {
                        ImGui::Text("Load Complete!");
                        isDialogOpen = false;  // Cerrar popup al finalizar
                    }
                    else if (loadFailed) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Load Failed: %s", loadError.c_str());
                    }
                }

                ImGui::EndPopup();
            }


            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 39, 512);
            ImGui::End();
        }



        void Shutdown() override
        {
            // Liberación de recursos si es necesario
            // Si los íconos son texturas de OpenGL, aquí deberías liberar esas texturas.
        }

    private:
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_DirectoryEntry;

        // Íconos (suponiendo que estos son texturas de OpenGL)
        GLuint iconFolder = 0;
        GLuint iconModel = 0;
        GLuint iconImage = 0;

        ImportModelData importOptions;
        bool isDialogOpen = false;
    };
}
