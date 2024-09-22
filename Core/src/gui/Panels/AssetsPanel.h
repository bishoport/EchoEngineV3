#pragma once
#include "PanelBase.h"
#include "../../managers/AssetsManager.h"

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
                        isModelDialogOpen = true;  // Abre el diálogo
                        m_DirectoryEntry = directoryEntry;
                    }
                    else if (path.extension() == ".jpg" || path.extension() == ".png") {
                        selectedFilePath = path.string(); // Guarda la ruta completa del archivo
                        isTextureDialogOpen = true;  // Abre el diálogo de selección de textura
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


            //--DIALOG IMPORT TEXTURE
            if (isTextureDialogOpen) {
                ImGui::OpenPopup("Texture Import Options");
            }

            if (ImGui::BeginPopupModal("Texture Import Options", &isTextureDialogOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
                // Mostrar la ruta al archivo seleccionado
                ImGui::Text("Ruta del archivo: %s", selectedFilePath.c_str());

                // Combo box para seleccionar el tipo de textura
                const char* textureTypes[] = { "ALBEDO", "NORMAL", "METALLIC", "ROUGHNESS", "AO" };
                static int currentItem = 0;  // Almacena el índice del tipo de textura seleccionado
                ImGui::Combo("Tipo de textura", &currentItem, textureTypes, IM_ARRAYSIZE(textureTypes));

                // Botón de Aceptar
                if (ImGui::Button("Aceptar")) {
                    // Convertir el índice seleccionado a TEXTURE_TYPES
                    selectedTextureType = static_cast<TEXTURE_TYPES>(currentItem);

                    // Cargar la textura con el tipo seleccionado
                    std::filesystem::path fullPath = selectedFilePath;
                    std::string filePath = fullPath.parent_path().string();
                    std::string fileName = fullPath.filename().string();

                    AssetsManager::GetInstance().LoadTextureAsset(selectedFilePath, filePath.c_str(), fileName.c_str(), selectedTextureType);

                    isTextureDialogOpen = false;  // Cerrar el diálogo
                }

                ImGui::SameLine();

                // Botón de Cancelar
                if (ImGui::Button("Cancelar")) {
                    isTextureDialogOpen = false;  // Cerrar el diálogo sin hacer nada
                }

                ImGui::EndPopup();
            }
            //---------------------------------------------------------------------------------------------------------------------------------------------------------


            if (isModelDialogOpen) {
                ImGui::OpenPopup("Model Import Options");
            }




            //--DIALOG IMPORT MODEL
            if (ImGui::BeginPopupModal("Model Import Options", &isModelDialogOpen))
            {
                // Obtener el fullPath y descomponer en filePath y fileName
                std::filesystem::path fullPath = m_DirectoryEntry;
                std::string filePath = fullPath.parent_path().string();
                std::string fileName = fullPath.filename().string();

                // Mostrar el path y el nombre de archivo
                ImGui::Text("Path: %s", filePath.c_str());
                ImGui::Text("File: %s", fileName.c_str());
                ImGui::Separator();

                // Mostrar las opciones de importación
                ImGui::Checkbox("Invert UV", &importOptions.invertUV);
                ImGui::Checkbox("Rotate 90 degrees on X-axis", &importOptions.rotate90);
                ImGui::SliderFloat("Scale", &importOptions.globalScaleFactor, 0.01f, 10.0f, "%.1f", ImGuiSliderFlags_None);
                ImGui::Separator();

                ImGui::Checkbox("Lights", &importOptions.processLights);

                // Botón de Aceptar
                if (ImGui::Button("Accept")) {
                    //isModelDialogOpen = false;  // Cierra el diálogo

                    if (filePath.back() != '\\' && filePath.back() != '/') {
                        filePath += '\\';  // Asegura que el filePath termina en '\'
                    }
                    importOptions.filePath = filePath;
                    importOptions.fileName = fileName;

                    AssetsManager::GetInstance().LoadModelAsset(importOptions);
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    isModelDialogOpen = false;  // Cierra el diálogo
                }
                ImGui::EndPopup();
            }
            //---------------------------------------------------------------------------------------------------------------------------------------------------------



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
        bool isTextureDialogOpen = false;
        std::string selectedFilePath;
        TEXTURE_TYPES selectedTextureType = TEXTURE_TYPES::ALBEDO;

        
        // Variables para el diálogo de modelos
        bool isModelDialogOpen = false;
        ImportModelData importOptions;
        
    };
}
