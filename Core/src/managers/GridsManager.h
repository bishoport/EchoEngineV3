#pragma once

#include "../core/Core.h"
#include <yaml-cpp/yaml.h>
#include <vector>
#include <variant>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

namespace libCore
{
    class GridsManager
    {
    public:
        using CellData = std::variant<int, float, std::string, bool, uint32_t>;



        // Singleton access
        static GridsManager& GetInstance()
        {
            static GridsManager instance;
            return instance;
        }



        bool SaveGridToFile(const std::vector<std::vector<CellData>>& gridData, const std::string& fileName, int width, int height, int dataType)
        {
            try {
                YAML::Emitter out;
                out << YAML::BeginMap;
                out << YAML::Key << "width" << YAML::Value << width;
                out << YAML::Key << "height" << YAML::Value << height;
                out << YAML::Key << "dataType" << YAML::Value << dataType;  // Guardar el tipo de dato de la capa
                out << YAML::Key << "mapData" << YAML::BeginSeq;

                for (const auto& row : gridData) {
                    out << YAML::BeginSeq;
                    for (const auto& cell : row) {
                        out << GetCellText(cell);  // Convertir CellData a texto
                    }
                    out << YAML::EndSeq;
                }
                out << YAML::EndSeq;
                out << YAML::EndMap;

                std::ofstream fout(fileName);
                fout << out.c_str();
                fout.close();

                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Error al guardar el grid: " << e.what() << std::endl;
                return false;
            }
        }


        // Función para cargar un archivo YAML y construir la matriz
        bool LoadGridFromYAML(const std::string& filePath, const std::string& fileName)
        {
            try
            {
                YAML::Node data = YAML::LoadFile(filePath);
                if (!data) return false;

                int width = data["width"].as<int>();
                int height = data["height"].as<int>();
                int dataType = data["dataType"].as<int>();  // Leer el tipo de dato de la capa

                std::vector<std::vector<CellData>> gridData;
                gridData.resize(height, std::vector<CellData>(width));

                int y = 0;
                for (const auto& rowNode : data["mapData"]) // Procesar los datos del mapa
                {
                    int x = 0;
                    for (const auto& cellNode : rowNode)
                    {
                        std::string valueStr = cellNode.as<std::string>();

                        // Asignar valores predeterminados o convertir el valor según el tipo de dato
                        SetCellValue(gridData[y][x], valueStr, dataType);
                        ++x;
                    }
                    ++y;
                }

                // Guardar el grid en el mapa
                m_grids[fileName] = gridData;

                return true;
            }
            catch (const YAML::Exception& e)
            {
                // Manejo de errores en la carga del archivo
                std::cerr << "Error al cargar el archivo YAML: " << e.what() << std::endl;
                return false;
            }
        }


        // Función para detectar el tipo de dato en base al contenido de la celda
        int DetectDataType(const std::string& valueStr)
        {
            if (valueStr == "true" || valueStr == "false") {
                return 3; // bool
            }

            try {
                std::stoi(valueStr);
                return 0; // int
            }
            catch (...) {}

            try {
                std::stof(valueStr);
                return 1; // float
            }
            catch (...) {}

            try {
                std::stoul(valueStr);
                return 4; // uint32_t
            }
            catch (...) {}

            return 2; // string
        }



        // Obtener un grid cargado
        const std::vector<std::vector<CellData>>& GetGrid(const std::string& fileName) const
        {
            return m_grids.at(fileName); // Se puede lanzar una excepción si el archivo no existe
        }

        bool IsGridLoaded(const std::string& fileName) const
        {
            return m_grids.find(fileName) != m_grids.end();
        }

        // Obtener un mapa con los grids cargados
        const std::unordered_map<std::string, std::vector<std::vector<CellData>>>& GetLoadedGrids() const
        {
            return m_grids;
        }


    private:
        // Constructor privado
        GridsManager() {}
        GridsManager(const GridsManager&) = delete;
        GridsManager& operator=(const GridsManager&) = delete;
        ~GridsManager() {}

        // Almacenar múltiples grids
        std::unordered_map<std::string, std::vector<std::vector<CellData>>> m_grids;

        // Función para convertir el valor de CellData a string
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

        // Función para establecer el valor de una celda desde una cadena
        void SetCellValue(CellData& cell, const std::string& valueStr, int dataType)
        {
            try
            {
                if (valueStr.empty()) {
                    // Asignar un valor predeterminado si la cadena está vacía
                    switch (dataType)
                    {
                    case 0: // int
                        cell = 0;
                        break;
                    case 1: // float
                        cell = 0.0f;
                        break;
                    case 2: // string
                        cell = std::string("");
                        break;
                    case 3: // bool
                        cell = false;
                        break;
                    case 4: // uint32_t
                        cell = static_cast<uint32_t>(0);
                        break;
                    default:
                        throw std::runtime_error("Tipo de dato desconocido en SetCellValue");
                    }
                }
                else {
                    // Si la cadena no está vacía, convertirla al tipo de dato adecuado
                    switch (dataType)
                    {
                    case 0: // int
                        cell = std::stoi(valueStr);
                        break;
                    case 1: // float
                        cell = std::stof(valueStr);
                        break;
                    case 2: // string
                        cell = valueStr;
                        break;
                    case 3: // bool
                        cell = (valueStr == "true");
                        break;
                    case 4: // uint32_t
                        cell = static_cast<uint32_t>(std::stoul(valueStr));
                        break;
                    default:
                        throw std::runtime_error("Tipo de dato desconocido en SetCellValue");
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error al convertir el valor de la celda: " << valueStr << " - " << e.what() << std::endl;
            }
        }



    };
}



//#pragma once
//
//#include "../core/Core.h"
//#include <vector>
//#include <variant>
//#include <yaml-cpp/yaml.h>
//#include <string>
//#include <unordered_map>
//#include <iostream>
//
//namespace libCore
//{
//    class GridsManager
//    {
//    public:
//        using CellData = std::variant<int, float, std::string, bool, uint32_t>;
//
//        // Singleton access
//        static GridsManager& GetInstance()
//        {
//            static GridsManager instance;
//            return instance;
//        }
//
//
//        // Función para cargar un archivo YAML y construir la matriz
//        bool LoadGridFromYAML(const std::string& filePath, const std::string& fileName)
//        {
//            try
//            {
//                YAML::Node data = YAML::LoadFile(filePath);
//                if (!data) return false;
//
//                int width = data["width"].as<int>();
//                int height = data["height"].as<int>();
//
//                std::vector<std::vector<CellData>> gridData;
//                gridData.resize(height, std::vector<CellData>(width));
//
//                int y = 0;
//                for (const auto& rowNode : data["layers"][0]["mapData"]) // Supongamos que quieres la primera capa
//                {
//                    int x = 0;
//                    for (const auto& cellNode : rowNode)
//                    {
//                        SetCellValue(gridData[y][x], cellNode.as<std::string>(), data["layers"][0]["dataType"].as<int>());
//                        ++x;
//                    }
//                    ++y;
//                }
//
//                // Guardar el grid en el mapa
//                m_grids[fileName] = gridData;
//
//                return true;
//            }
//            catch (const YAML::Exception& e)
//            {
//                // Manejo de errores en la carga del archivo
//                std::cerr << "Error al cargar el archivo YAML: " << e.what() << std::endl;
//                return false;
//            }
//        }
//
//        // Obtener el valor de una celda específica de un grid en particular
//        CellData GetCellValue(const std::string& fileName, int x, int y) const
//        {
//            if (m_grids.find(fileName) != m_grids.end())
//            {
//                const auto& grid = m_grids.at(fileName);
//                int width = grid[0].size();
//                int height = grid.size();
//
//                if (x >= 0 && x < width && y >= 0 && y < height)
//                {
//                    return grid[y][x];
//                }
//                else
//                {
//                    throw std::out_of_range("Índice fuera de los límites.");
//                }
//            }
//            else
//            {
//                throw std::invalid_argument("El grid solicitado no se encuentra.");
//            }
//        }
//
//        // Verifica si un grid ha sido cargado
//        bool IsGridLoaded(const std::string& fileName) const
//        {
//            return m_grids.find(fileName) != m_grids.end();
//        }
//
//    private:
//        // Constructor privado
//        GridsManager() {}
//        GridsManager(const GridsManager&) = delete;
//        GridsManager& operator=(const GridsManager&) = delete;
//        ~GridsManager() {}
//
//        // Almacenar múltiples grids en un unordered_map
//        std::unordered_map<std::string, std::vector<std::vector<CellData>>> m_grids;
//
//        // Función para establecer el valor de una celda desde una cadena
//        void SetCellValue(CellData& cell, const std::string& valueStr, int dataType)
//        {
//            try
//            {
//                switch (dataType)
//                {
//                case 0: // int
//                    cell = std::stoi(valueStr);
//                    break;
//                case 1: // float
//                    cell = std::stof(valueStr);
//                    break;
//                case 2: // string
//                    cell = valueStr;
//                    break;
//                case 3: // bool
//                    cell = (valueStr == "true");
//                    break;
//                case 4: // uint32_t
//                    cell = static_cast<uint32_t>(std::stoul(valueStr));
//                    break;
//                }
//            }
//            catch (...)
//            {
//                // Manejo de errores en la conversión de tipos
//                std::cerr << "Error al convertir el valor de la celda: " << valueStr << std::endl;
//            }
//        }
//    };
//}
