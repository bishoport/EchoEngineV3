#pragma once
#include "PanelBase.h"
#include "../../managers/AssetsManager.h"
#include "../../managers/SceneManager.h"

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

            //PopUps
            // Registrar el popup de importación de texturas
            PopupManager::GetInstance().RegisterPopup("Texture Import Options", [this]() {
                ImGui::Text("Ruta del archivo: %s %s", ICON_FA_FOLDER_OPEN, selectedFilePath.c_str());

                const char* textureTypes[] = { ICON_FA_PAINT_BRUSH " ALBEDO", ICON_FA_SUN " NORMAL", ICON_FA_GEM " METALLIC", ICON_FA_ROAD " ROUGHNESS", ICON_FA_ADJUST " AO" };
                static int currentItem = 0;  // Almacena el índice del tipo de textura seleccionado
                ImGui::Combo("Tipo de textura", &currentItem, textureTypes, IM_ARRAYSIZE(textureTypes));

                if (ImGui::Button(ICON_FA_CHECK " Aceptar")) {
                    selectedTextureType = static_cast<TEXTURE_TYPES>(currentItem);

                    std::filesystem::path fullPath = selectedFilePath;
                    std::string filePath = fullPath.parent_path().string();
                    std::string fileName = fullPath.filename().string();

                    AssetsManager::GetInstance().LoadTextureAsset(selectedFilePath, filePath.c_str(), fileName.c_str(), selectedTextureType);

                    PopupManager::GetInstance().ClosePopup("Texture Import Options");
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_TIMES " Cancelar")) {
                    PopupManager::GetInstance().ClosePopup("Texture Import Options");
                }
            });
            // Registrar el popup de importación de modelos
            PopupManager::GetInstance().RegisterPopup("Model Import Options", [this]() {
                std::filesystem::path fullPath = m_DirectoryEntry;
                std::string filePath = fullPath.parent_path().string();
                std::string fileName = fullPath.filename().string();

                ImGui::Text("Path: %s %s", ICON_FA_FOLDER_OPEN, filePath.c_str());
                ImGui::Text("File: %s %s", ICON_FA_FILE, fileName.c_str());
                ImGui::Separator();

                ImGui::Checkbox(ICON_FA_CHECK_SQUARE " Invert UV", &importOptions.invertUV);
                ImGui::Checkbox(ICON_FA_SYNC " Rotate 90 degrees on X-axis", &importOptions.rotate90);
                ImGui::SliderFloat(ICON_FA_EXPAND " Scale", &importOptions.globalScaleFactor, 0.1f, 10.0f, "%.1f");
                ImGui::Separator();

                ImGui::Checkbox(ICON_FA_LIGHTBULB " Lights", &importOptions.processLights);

                if (ImGui::Button(ICON_FA_CHECK " Accept")) {
                    if (filePath.back() != '\\' && filePath.back() != '/') {
                        filePath += '\\';  // Asegura que el filePath termina en '\'
                    }
                    importOptions.filePath = filePath;
                    importOptions.fileName = fileName;

                    AssetsManager::GetInstance().LoadModelAsset(importOptions);
                    PopupManager::GetInstance().ClosePopup("Model Import Options");
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_TIMES " Cancel")) {
                    PopupManager::GetInstance().ClosePopup("Model Import Options");
                }
            });

        }

        void Draw() override
        {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            ImGui::BeginChild("FileRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

            if (m_CurrentDirectory != std::filesystem::path("assets")) {
                if (ImGui::Button(ICON_FA_ARROW_LEFT " Back")) {
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

                const char* iconLabel = ICON_FA_FILE;

                if (directoryEntry.is_directory()) {
                    iconLabel = ICON_FA_FOLDER;
                }
                else if (path.extension() == ".fbx" || path.extension() == ".obj" || path.extension() == ".gltf" || path.extension() == ".glb") {
                    iconLabel = ICON_FA_CUBE;
                }
                else if (path.extension() == ".png" || path.extension() == ".jpg") {
                    iconLabel = ICON_FA_IMAGE;
                }
                else if (path.extension() == ".lua") {
                    iconLabel = ICON_FA_CODE;
                }

                ImVec2 iconSize(thumbnailSize, thumbnailSize);

                if (ImGui::Button(iconLabel, iconSize)) {
                    if (directoryEntry.is_directory()) {
                        m_CurrentDirectory /= path.filename();
                    }
                    else if (path.extension() == ".lua") {
                        std::string scriptName = path.stem().string();
                        LuaManager::GetInstance().LoadLuaFile(scriptName, path.string());
                    }
                    else if (path.extension() == ".fbx" || path.extension() == ".obj" || path.extension() == ".gltf" || path.extension() == ".glb") {
                        m_DirectoryEntry = directoryEntry;
                        PopupManager::GetInstance().OpenPopup("Model Import Options");
                    }
                    else if (path.extension() == ".jpg" || path.extension() == ".png") {
                        selectedFilePath = path.string();
                        PopupManager::GetInstance().OpenPopup("Texture Import Options");
                    }
                    else if (path.extension() == ".yaml" || path.extension() == ".yml")
                    {
                        // Aquí llamas a la función para deserializar la escena
                        std::string sceneName = path.stem().string(); // Obtén el nombre del archivo sin extensión
                        SceneManager::GetInstance().GetCurrentScene()->DeserializeScene(sceneName);
                    }
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0, 0.0f)) {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                        const std::string payload_n = relativePath.string();
                        ImGui::SetDragDropPayload("ASSET_DRAG", payload_n.c_str(), payload_n.length() + 1, ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }
                }

                ImGui::TextWrapped(filenameString.c_str());
                ImGui::NextColumn();

                ImGui::PopID();
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

        //Iconos (suponiendo que estos son texturas de OpenGL)
        GLuint iconFolder = 0;
        GLuint iconModel = 0;
        GLuint iconImage = 0;

        //Paths
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_DirectoryEntry;

        // Variables para el diálogo de textura
        //bool isTextureDialogOpen = false;
        std::string selectedFilePath;
        TEXTURE_TYPES selectedTextureType = TEXTURE_TYPES::ALBEDO;

        
        // Variables para el diálogo de modelos
        //bool isModelDialogOpen = false;
        ImportModelData importOptions;
        
    };
}
