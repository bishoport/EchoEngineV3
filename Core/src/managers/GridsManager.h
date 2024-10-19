#pragma once

#include "../core/Core.h"
#include <vector>
#include <variant>
#include <yaml-cpp/yaml.h>
#include <string>
#include <unordered_map>
#include <iostream>

namespace libCore
{
    using CellData = std::variant<int, float, std::string, bool, uint32_t>;

    struct LayerGridData
    {
        enum class DataType { INT, FLOAT, STRING, BOOL, UINT32 };
        std::string name;
        DataType dataType = DataType::INT;
        CellData defaultValue = 0;
        std::vector<std::vector<CellData>> mapData;
    };

    struct GridData
    {
        std::string name;
        std::string filepath;
        std::map<std::string, LayerGridData> layers;
    };

    class GridsManager
    {
    public:
        // Singleton access
        static GridsManager& GetInstance()
        {
            static GridsManager instance;
            return instance;
        }


        bool LoadGridFromYAML(const std::string& filePath, const std::string& fileName)
        {
            try
            {
                YAML::Node data = YAML::LoadFile(filePath);
                if (!data) return false;

                int width = data["width"].as<int>();
                int height = data["height"].as<int>();

                Ref<GridData> grid;

                // Verificar si el grid ya existe
                auto it = m_grids.find(fileName);
                if (it != m_grids.end()) {
                    // Si el grid ya existe, simplemente actualizamos sus valores
                    grid = it->second;
                    std::cout << "Actualizando grid existente: " << fileName << std::endl;
                }
                else {
                    // Si no existe, creamos uno nuevo
                    grid = CreateRef<GridData>();
                    grid->name = fileName;
                    grid->filepath = filePath;
                    std::cout << "Cargando nuevo grid: " << fileName << std::endl;
                }

                // Limpiar capas anteriores (si existen)
                grid->layers.clear();

                // Iterar sobre las capas en el YAML
                for (const auto& layerNode : data["layers"]) {
                    LayerGridData layer;
                    layer.name = layerNode["name"].as<std::string>();
                    layer.dataType = static_cast<LayerGridData::DataType>(layerNode["dataType"].as<int>());

                    SetDefaultValueForLayer(layer); // Establecer el valor predeterminado
                    layer.mapData.resize(height, std::vector<CellData>(width, layer.defaultValue));

                    int y = 0;
                    for (const auto& rowNode : layerNode["mapData"]) {
                        int x = 0;
                        for (const auto& cellNode : rowNode) {
                            SetCellValue(layer.mapData[y][x], cellNode.as<std::string>(), layer.dataType);
                            ++x;
                        }
                        ++y;
                    }

                    // Asignar la capa al grid
                    grid->layers[layer.name] = layer;
                }

                // Guardar o actualizar el grid en el mapa
                m_grids[fileName] = grid;

                // Depuración opcional: imprime el grid cargado
                GridsManager::GetInstance().PrintGrid(fileName);
                return true;
            }
            catch (const YAML::Exception& e)
            {
                // Manejo de errores en la carga del archivo
                std::cerr << "Error al cargar el archivo YAML GRID: " << e.what() << std::endl;
                return false;
            }
        }




        Ref<GridData> GetGrid(const std::string& gridName) const
        {
            auto it = m_grids.find(gridName);
            if (it != m_grids.end()) {
                return it->second;
            }
            return nullptr; // Si no se encuentra el grid
        }

        std::vector<std::string> GetGridNames() const
        {
            std::vector<std::string> gridNames;
            for (const auto& [name, grid] : m_grids) {
                gridNames.push_back(name);
            }
            return gridNames;
        }


        std::optional<std::vector<std::vector<int>>> GetLayerDataAsLuaMatrix(const std::string& gridName, const std::string& layerName)
        {
            // Verificar si el grid existe
            auto gridIt = m_grids.find(gridName);
            if (gridIt == m_grids.end()) {
                std::cerr << "Error: Grid '" << gridName << "' no encontrado." << std::endl;
                return std::nullopt;
            }

            Ref<GridData> grid = gridIt->second;

            // Verificar si la capa existe en el grid
            auto layerIt = grid->layers.find(layerName);
            if (layerIt == grid->layers.end()) {
                std::cerr << "Error: Capa '" << layerName << "' no encontrada en el Grid '" << gridName << "'." << std::endl;
                return std::nullopt;
            }

            const LayerGridData& layer = layerIt->second;

            // Verificar que el tipo de datos de la capa sea INT (u otro tipo que desees usar)
            if (layer.dataType != LayerGridData::DataType::INT) {
                std::cerr << "Error: La capa '" << layerName << "' en el Grid '" << gridName << "' no contiene datos de tipo INT." << std::endl;
                return std::nullopt;
            }

            // Crear una matriz de ints a partir de la capa
            std::vector<std::vector<int>> luaMatrix;
            luaMatrix.resize(layer.mapData.size());

            for (size_t y = 0; y < layer.mapData.size(); ++y) {
                luaMatrix[y].resize(layer.mapData[y].size());
                for (size_t x = 0; x < layer.mapData[y].size(); ++x) {
                    luaMatrix[y][x] = std::get<int>(layer.mapData[y][x]);  // Extraer los valores int
                }
            }

            return luaMatrix;
        }

    private:
        // Constructor privado
        GridsManager() {}
        GridsManager(const GridsManager&) = delete;
        GridsManager& operator=(const GridsManager&) = delete;
        ~GridsManager() {}

        // Almacenar múltiples grids en un unordered_map
        std::unordered_map<std::string, Ref<GridData>> m_grids;

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
        void SetCellValue(CellData& cell, const std::string& valueStr, LayerGridData::DataType dataType)
        {
            try {
                switch (dataType)
                {
                case LayerGridData::DataType::INT:
                    cell = std::stoi(valueStr);
                    break;
                case LayerGridData::DataType::FLOAT:
                    cell = std::stof(valueStr);
                    break;
                case LayerGridData::DataType::STRING:
                    cell = valueStr;
                    break;
                case LayerGridData::DataType::BOOL:
                    cell = (valueStr == "true");
                    break;
                case LayerGridData::DataType::UINT32:
                    cell = static_cast<uint32_t>(std::stoul(valueStr));
                    break;
                }
            }
            catch (...) {
                // Handle invalid conversions gracefully
            }
        }


        std::string DataTypeToString(LayerGridData::DataType dataType)
        {
            switch (dataType)
            {
            case LayerGridData::DataType::INT:
                return "INT";
            case LayerGridData::DataType::FLOAT:
                return "FLOAT";
            case LayerGridData::DataType::STRING:
                return "STRING";
            case LayerGridData::DataType::BOOL:
                return "BOOL";
            case LayerGridData::DataType::UINT32:
                return "UINT32";
            default:
                return "UNKNOWN";
            }
        }



        



        void PrintGrid(const std::string& gridName)
        {
            // Buscar el grid en el mapa
            auto it = m_grids.find(gridName);
            if (it == m_grids.end()) {
                std::cout << "Grid '" << gridName << "' no encontrado." << std::endl;
                return;
            }

            Ref<GridData> grid = it->second;

            std::cout << "Grid: " << grid->name << std::endl;
            std::cout << "File path: " << grid->filepath << std::endl;

            // Recorrer todas las capas del grid
            for (const auto& [layerName, layer] : grid->layers) {
                std::cout << "Layer: " << layer.name << " (" << DataTypeToString(layer.dataType) << ")" << std::endl;

                // Recorrer todas las filas y columnas de la capa
                for (size_t y = 0; y < layer.mapData.size(); ++y) {
                    for (size_t x = 0; x < layer.mapData[y].size(); ++x) {
                        // Imprimir el valor según el tipo de dato de la capa
                        std::visit([](auto&& value) {
                            std::cout << value << " ";
                            }, layer.mapData[y][x]);
                    }
                    std::cout << std::endl; // Nueva línea al final de cada fila
                }
                std::cout << std::endl; // Separador entre capas
            }

        }

    };
}
