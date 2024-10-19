#pragma once
#include "PanelBase.h"
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "../../managers/GridsManager.h"
#include <array>

namespace libCore
{
    class MapEditorPanel : public PanelBase
    {
    public:

        MapEditorPanel()
            : PanelBase("Map Editor"), m_width(10), m_height(10), m_zoom(1.0f), m_currentLayer(0), m_scrollX(0.0f), m_scrollY(0.0f), m_filePath("no_named_map")
        {
            AddLayer("Default Layer"); // Agregar una capa por defecto
            m_selectedCell = { 0, 0 }; // Seleccionar la primera celda al inicio

            InitColorPalette();
        }

        void Init() override{}

        void Draw() override
        {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());

            // Obtener el tamaño disponible de la ventana
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            // Calcular alturas de las áreas
            float topAreaHeight = windowSize.y * 0.2f;
            float bottomAreaHeight = windowSize.y - topAreaHeight;

            // --- Parte Superior (20% de altura) ---
            ImGui::BeginChild("TopArea", ImVec2(windowSize.x, topAreaHeight), false);

            // Dividir en dos columnas al 50% de ancho
            ImGui::Columns(2, nullptr, false);

            // Columna izquierda: sliders
            ImGui::SetColumnWidth(0, windowSize.x * 0.5f); // 50% de la columna izquierda
            ImGui::Text(ICON_FA_ARROWS_ALT " Map Settings:");
            ImGui::SliderInt(ICON_FA_ARROWS_ALT_H " Width", &m_width, 1, 100);
            UpdateMapSize(); // Actualizar tamaño después de modificar el ancho

            ImGui::SliderInt(ICON_FA_ARROWS_ALT_V " Height", &m_height, 1, 100);
            UpdateMapSize(); // Actualizar tamaño después de modificar la altura

            ImGui::Text(ICON_FA_ARROWS_ALT " Visualization:");
            ImGui::SliderFloat(ICON_FA_SEARCH_PLUS " Zoom", &m_zoom, 0.1f, 3.0f);
            UpdateMapSize(); // Actualizar tamaño después de modificar la altura

            ImGui::NextColumn();

            // Columna derecha: controles de carga/guardado
            ImGui::SetColumnWidth(1, windowSize.x * 0.5f); // 50% de la columna derecha
            ImGui::Text(ICON_FA_SAVE "File:");

            // Copiar el valor actual a un buffer de 256 caracteres
            char filePathBuffer[256];
            strncpy(filePathBuffer, m_filePath.c_str(), sizeof(filePathBuffer));

            // InputText con el nuevo buffer de 256 caracteres
            if (ImGui::InputText(ICON_FA_FILE " File name", filePathBuffer, sizeof(filePathBuffer))) {
                m_filePath = std::string(filePathBuffer); // Actualizar m_filePath después de la edición
            }

            if (ImGui::Button(ICON_FA_SAVE " Save Grid")) {
                SaveMap(m_filePath);
                GridsManager::GetInstance().LoadGridFromYAML(defaultAssetsPathGrid + "/" + m_filePath + extensionGridFile, m_filePath);
            }

            // COMBO
            std::vector<std::string> gridNames = GridsManager::GetInstance().GetGridNames();
            static int selectedGridIndex = -1;

            ImGui::Text("Loaded Grids:");

            // ComboBox para seleccionar un grid
            if (ImGui::BeginCombo("##GridCombo", selectedGridIndex >= 0 ? gridNames[selectedGridIndex].c_str() : "Select a Grid")) {
                for (int i = 0; i < gridNames.size(); ++i) {
                    bool isSelected = (selectedGridIndex == i);
                    if (ImGui::Selectable(gridNames[i].c_str(), isSelected)) {
                        selectedGridIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Colocar el botón en la misma línea
            ImGui::SameLine();

            // Botón para cargar el grid seleccionado
            if (ImGui::Button(ICON_FA_FOLDER_OPEN " Load Grid") && selectedGridIndex >= 0) {
                LoadGridToGUI(gridNames[selectedGridIndex]);
            }

            ImGui::Columns(1); // Finalizar columnas
            ImGui::Separator();

            ImGui::EndChild(); // Finalizar la parte superior

            // --- Parte Inferior (80% de altura) ---
            ImGui::BeginChild("BottomArea", ImVec2(windowSize.x, bottomAreaHeight), false);

            // Dividir en dos columnas para la parte inferior
            ImGui::Columns(2, nullptr, false);

            // Columna izquierda: controles de capas y el inspector de celdas (20% de ancho)
            ImGui::SetColumnWidth(0, windowSize.x * 0.2f); // 20% de la columna izquierda
            ImGui::BeginChild("LayerControls", ImVec2(0, 0), true);
            ImGui::Text(ICON_FA_LAYER_GROUP " Layer Controls:");

            // Botones para agregar y eliminar capas
            if (ImGui::Button(ICON_FA_PLUS " Add")) {
                AddLayer("New Layer " + std::to_string(m_mapLayers.size() + 1));
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_MINUS " Remove") && m_mapLayers.size() > 1) {
                ImGui::OpenPopup("Confirm Delete Layer");
            }

            // Confirmación para eliminar capa
            if (ImGui::BeginPopupModal("Confirm Delete Layer", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure you want to delete the current layer?\nThis operation cannot be undone!");
                ImGui::Separator();
                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    RemoveCurrentLayer();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Selección de capa
            ImGui::Text(ICON_FA_LAYER_GROUP " Current Layer:");
            if (ImGui::BeginCombo("##LayerCombo", m_mapLayers[m_currentLayer].name.c_str())) {
                for (int i = 0; i < m_mapLayers.size(); ++i) {
                    bool isSelected = (m_currentLayer == i);
                    if (ImGui::Selectable(m_mapLayers[i].name.c_str(), isSelected)) {
                        m_currentLayer = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();

            // Selección del tipo de dato de las celdas
            ImGui::Text(ICON_FA_COGS " Layer Properties:");
            char buffer[256];
            strncpy(buffer, m_mapLayers[m_currentLayer].name.c_str(), sizeof(buffer));
            if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
                m_mapLayers[m_currentLayer].name = buffer;
            }

            const char* dataTypes[] = { "int", "float", "string", "bool", "uint32_t" };
            if (ImGui::BeginCombo("Cell Data Type", dataTypes[static_cast<int>(m_mapLayers[m_currentLayer].dataType)])) {
                for (int n = 0; n < IM_ARRAYSIZE(dataTypes); n++) {
                    bool isSelected = (static_cast<int>(m_mapLayers[m_currentLayer].dataType) == n);
                    if (ImGui::Selectable(dataTypes[n], isSelected)) {
                        m_mapLayers[m_currentLayer].dataType = static_cast<LayerGridData::DataType>(n);
                        ResetLayerData(m_mapLayers[m_currentLayer]); // Resetear los datos de la capa al cambiar el tipo
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Mostrar las propiedades de la celda seleccionada
            if (m_selectedCell.has_value()) {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text(ICON_FA_CUBE " Selected Cell Properties:");
                ImGui::Text("Cell (%d, %d):", m_selectedCell->first, m_selectedCell->second);
                auto& cell = m_mapLayers[m_currentLayer].mapData[m_selectedCell->second][m_selectedCell->first];
                EditCellValue(cell, m_mapLayers[m_currentLayer].dataType);
            }

            ImGui::EndChild();

            ImGui::NextColumn();

            // Columna derecha: matriz de botones (80% de ancho)
            ImGui::SetColumnWidth(1, windowSize.x * 0.8f); // 80% de la columna derecha
            ImGui::BeginChild("MapArea", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::SetScrollX(m_scrollX);
            ImGui::SetScrollY(m_scrollY);

            for (int y = 0; y < m_height; ++y)
            {
                for (int x = 0; x < m_width; ++x)
                {
                    ImGui::PushID(y * m_width + x); // Asegurarse de que cada celda tenga un ID único

                    // Obtener la celda actual
                    auto& cell = m_mapLayers[m_currentLayer].mapData[y][x];
                    std::string cellText = GetCellText(cell);

                    // Limitar el texto al tamaño del botón
                    if (cellText.length() > 4) {
                        cellText = cellText.substr(0, 4) + "...";
                    }

                    // Color por defecto del botón
                    ImVec4 buttonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];

                    // Asignar color si es un valor int, uint32 o bool
                    if (m_mapLayers[m_currentLayer].dataType == LayerGridData::DataType::INT ||
                        m_mapLayers[m_currentLayer].dataType == LayerGridData::DataType::UINT32 ||
                        m_mapLayers[m_currentLayer].dataType == LayerGridData::DataType::BOOL) {

                        int value = std::visit([](auto&& arg) -> int {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, int> || std::is_same_v<T, uint32_t> || std::is_same_v<T, bool>)
                                return static_cast<int>(arg);
                            return -1; // Valor por defecto si no es de tipo esperado
                            }, cell);

                        // Verificar si el valor está entre 0 y 19 para asignar un color
                        if (value >= 0 && value < 20) {
                            buttonColor = m_colorPalette[value]; // Usar la paleta de colores
                        }
                    }

                    // Aplicar el color al botón
                    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);

                    // Mostrar el botón de la celda
                    if (ImGui::Button(cellText.c_str(), ImVec2(30 * m_zoom, 30 * m_zoom)))
                    {
                        // Al hacer clic, seleccionar la celda
                        m_selectedCell = { x, y };
                    }

                    // Mostrar menú emergente para copiar/pegar
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Copy")) {
                            m_clipboardCell = cell; // Copiar el valor de la celda seleccionada
                        }
                        if (ImGui::MenuItem("Paste", nullptr, false, m_clipboardCell.has_value())) {
                            cell = m_clipboardCell.value(); // Pegar el valor almacenado en el portapapeles
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopStyleColor(); // Restablecer el color por defecto
                    ImGui::PopID(); // Finalizar el ID de la celda

                    // Espaciado entre botones
                    if (x < m_width - 1)
                        ImGui::SameLine();
                }
            }



            m_scrollX = ImGui::GetScrollX();
            m_scrollY = ImGui::GetScrollY();

            ImGui::EndChild(); // Finalizar la parte de la matriz de botones

            ImGui::Columns(1); // Finalizar columnas

            ImGui::EndChild(); // Finalizar la parte inferior

            ImGui::End(); // Finalizar ventana principal
        }

        void Shutdown() override
        {
            // Liberación de recursos si es necesario
        }





    private:
        std::string defaultAssetsPathGrid = "assets/Grids";
        std::string extensionGridFile = ".grid";
        int m_width;
        int m_height;
        float m_zoom;
        int m_currentLayer;
        float m_scrollX;
        float m_scrollY;
        std::vector<LayerGridData> m_mapLayers;
        std::optional<std::pair<int, int>> m_selectedCell;
        std::optional<CellData> m_clipboardCell;
        std::string m_filePath;
        std::array<ImVec4, 20> m_colorPalette; // Paleta de colores


        //--LAYERS MANAGMENT
        void AddLayer(const std::string& name)
        {
            LayerGridData newLayer;
            newLayer.name = name;
            SetDefaultValueForLayer(newLayer);
            newLayer.mapData.resize(m_height, std::vector<CellData>(m_width, newLayer.defaultValue));
            m_mapLayers.push_back(newLayer);
        }
        void RemoveCurrentLayer()
        {
            if (m_mapLayers.size() > 1)
            {
                m_mapLayers.erase(m_mapLayers.begin() + m_currentLayer);
                m_currentLayer = std::max(0, m_currentLayer - 1);
            }
        }

        //--CELL DATA MANAGMENT
        bool EditCellValue(CellData& cell, LayerGridData::DataType dataType)
        {
            bool valueChanged = false;
            switch (dataType)
            {
            case LayerGridData::DataType::INT: {
                int value = std::get<int>(cell);
                if (ImGui::InputInt("Value", &value)) {
                    cell = value;
                    valueChanged = true;
                }
                break;
            }
            case LayerGridData::DataType::FLOAT: {
                float value = std::get<float>(cell);
                if (ImGui::InputFloat("Value", &value)) {
                    cell = value;
                    valueChanged = true;
                }
                break;
            }
            case LayerGridData::DataType::STRING: {
                std::string value = std::get<std::string>(cell);
                char buffer[256];
                strncpy(buffer, value.c_str(), sizeof(buffer));
                if (ImGui::InputText("Value", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    cell = std::string(buffer);
                    valueChanged = true;
                }
                break;
            }
            case LayerGridData::DataType::BOOL: {
                bool value = std::get<bool>(cell);
                if (ImGui::Checkbox("Value", &value)) {
                    cell = value;
                    valueChanged = true;
                }
                break;
            }
            case LayerGridData::DataType::UINT32: {
                uint32_t value = std::get<uint32_t>(cell);
                int temp = static_cast<int>(value);
                if (ImGui::InputInt("Value", &temp)) {
                    cell = static_cast<uint32_t>(temp);
                    valueChanged = true;
                }
                break;
            }
            }
            return valueChanged;
        }
        void SetDefaultValueForLayer(LayerGridData& layer)
        {
            switch (layer.dataType)
            {
            case LayerGridData::DataType::INT:
                layer.defaultValue = 0;
                break;
            case LayerGridData::DataType::FLOAT:
                layer.defaultValue = 0.0f;
                break;
            case LayerGridData::DataType::STRING:
                layer.defaultValue = std::string("");
                break;
            case LayerGridData::DataType::BOOL:
                layer.defaultValue = false;
                break;
            case LayerGridData::DataType::UINT32:
                layer.defaultValue = static_cast<uint32_t>(0);
                break;
            }
        }
        void ResetLayerData(LayerGridData& layer)
        {
            SetDefaultValueForLayer(layer);
            for (auto& row : layer.mapData)
            {
                std::fill(row.begin(), row.end(), layer.defaultValue);
            }
        }

        //--UI
        std::string GetCellText(const CellData& cell) const
        {
            return std::visit([](auto&& value) -> std::string {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, int>) return std::to_string(value);
                else if constexpr (std::is_same_v<T, float>) return std::to_string(value);
                else if constexpr (std::is_same_v<T, std::string>) return value;
                else if constexpr (std::is_same_v<T, bool>) return value ? "true" : "false";
                else if constexpr (std::is_same_v<T, uint32_t>) return std::to_string(value);
                return "";
                }, cell);
        }
        void UpdateMapSize() {
            for (auto& layer : m_mapLayers) {
                layer.mapData.resize(m_height);
                for (auto& row : layer.mapData) {
                    row.resize(m_width, layer.defaultValue);
                }
            }
        }

        //--COLOR PALETTE
        void InitColorPalette()
        {
            m_colorPalette = {
                ImVec4(0.1f, 0.1f, 0.9f, 1.0f), // Color for 0
                ImVec4(0.2f, 0.2f, 0.8f, 1.0f), // Color for 1
                ImVec4(0.3f, 0.3f, 0.7f, 1.0f), // Color for 2
                ImVec4(0.4f, 0.4f, 0.6f, 1.0f), // Color for 3
                ImVec4(0.5f, 0.5f, 0.5f, 1.0f), // Color for 4
                ImVec4(0.6f, 0.6f, 0.4f, 1.0f), // Color for 5
                ImVec4(0.7f, 0.7f, 0.3f, 1.0f), // Color for 6
                ImVec4(0.8f, 0.8f, 0.2f, 1.0f), // Color for 7
                ImVec4(0.9f, 0.9f, 0.1f, 1.0f), // Color for 8
                ImVec4(1.0f, 0.1f, 0.1f, 1.0f), // Color for 9
                ImVec4(1.0f, 0.2f, 0.2f, 1.0f), // Color for 10
                ImVec4(1.0f, 0.3f, 0.3f, 1.0f), // Color for 11
                ImVec4(1.0f, 0.4f, 0.4f, 1.0f), // Color for 12
                ImVec4(1.0f, 0.5f, 0.5f, 1.0f), // Color for 13
                ImVec4(1.0f, 0.6f, 0.6f, 1.0f), // Color for 14
                ImVec4(1.0f, 0.7f, 0.7f, 1.0f), // Color for 15
                ImVec4(1.0f, 0.8f, 0.8f, 1.0f), // Color for 16
                ImVec4(1.0f, 0.9f, 0.9f, 1.0f), // Color for 17
                ImVec4(0.9f, 0.9f, 1.0f, 1.0f), // Color for 18
                ImVec4(0.8f, 0.8f, 1.0f, 1.0f)  // Color for 19
            };
        }

        //LOADER FROM GRIDSMANAGER
        void LoadGridToGUI(const std::string& gridName)
        {
            auto& gridsManager = GridsManager::GetInstance();
            auto grid = gridsManager.GetGrid(gridName);
            if (!grid) {
                std::cerr << "Error: Grid not found." << std::endl;
                return;
            }

            // Actualizar las dimensiones del mapa
            m_width = grid->layers.begin()->second.mapData[0].size();
            m_height = grid->layers.begin()->second.mapData.size();

            m_filePath = grid->name;

            // Limpiar capas actuales y reemplazar con las del grid cargado
            m_mapLayers.clear();
            for (const auto& [layerName, layer] : grid->layers) {
                LayerGridData newLayer;
                newLayer.name = layer.name;
                newLayer.dataType = layer.dataType;
                newLayer.mapData = layer.mapData;
                m_mapLayers.push_back(newLayer);
            }

            std::cout << "Grid '" << gridName << "' loaded into the editor." << std::endl;
        }

        //--SAVE GRID
        void SaveMap(const std::string& filePath)
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "width" << YAML::Value << m_width;
            out << YAML::Key << "height" << YAML::Value << m_height;
            out << YAML::Key << "layers" << YAML::BeginSeq;
            for (const auto& layer : m_mapLayers) {
                out << YAML::BeginMap;
                out << YAML::Key << "name" << YAML::Value << layer.name;
                out << YAML::Key << "dataType" << YAML::Value << static_cast<int>(layer.dataType);
                out << YAML::Key << "mapData" << YAML::BeginSeq;
                for (const auto& row : layer.mapData) {
                    out << YAML::BeginSeq;
                    for (const auto& cell : row) {
                        out << GetCellText(cell);
                    }
                    out << YAML::EndSeq;
                }
                out << YAML::EndSeq;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream fout(defaultAssetsPathGrid + "/" + filePath + extensionGridFile);
            fout << out.c_str();
        }
        //---------------------------------------------------------------------------------------
    };
}



//ImGui::SameLine();
        //if (ImGui::Button(ICON_FA_FOLDER_OPEN " Load Map")) {
        //    LoadMap(m_filePath);
        //}

        ////--LOAD GRID
        //void LoadMap(const std::string& filePath)
        //{
        //    YAML::Node data = YAML::LoadFile(filePath);
        //    if (!data) return;

        //    m_width = data["width"].as<int>();
        //    m_height = data["height"].as<int>();
        //    m_mapLayers.clear();

        //    for (const auto& layerNode : data["layers"]) {
        //        LayerGridData layer;
        //        layer.name = layerNode["name"].as<std::string>();
        //        layer.dataType = static_cast<LayerGridData::DataType>(layerNode["dataType"].as<int>());
        //        SetDefaultValueForLayer(layer); // Aquí establecemos el valor predeterminado
        //        layer.mapData.resize(m_height, std::vector<CellData>(m_width, layer.defaultValue));

        //        int y = 0;
        //        for (const auto& rowNode : layerNode["mapData"]) {
        //            int x = 0;
        //            for (const auto& cellNode : rowNode) {
        //                SetCellValue(layer.mapData[y][x], cellNode.as<std::string>(), layer.dataType);
        //                ++x;
        //            }
        //            ++y;
        //        }
        //        m_mapLayers.push_back(layer);
        //    }
        //}
        //void SetCellValue(CellData& cell, const std::string& valueStr, LayerGridData::DataType dataType)
        //{
        //    try {
        //        switch (dataType)
        //        {
        //        case LayerGridData::DataType::INT:
        //            cell = std::stoi(valueStr);
        //            break;
        //        case LayerGridData::DataType::FLOAT:
        //            cell = std::stof(valueStr);
        //            break;
        //        case LayerGridData::DataType::STRING:
        //            cell = valueStr;
        //            break;
        //        case LayerGridData::DataType::BOOL:
        //            cell = (valueStr == "true");
        //            break;
        //        case LayerGridData::DataType::UINT32:
        //            cell = static_cast<uint32_t>(std::stoul(valueStr));
        //            break;
        //        }
        //    }
        //    catch (...) {
        //        // Handle invalid conversions gracefully
        //    }
        //}
        //--------------------------------------------------------------------------------------------------------