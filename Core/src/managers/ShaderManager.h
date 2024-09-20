#pragma once

#include "../core/Core.h"
#include "../core/shaders/Shader.h"

namespace libCore 
{
    class ShaderManager {
    public:

        static ShaderManager& GetInstance() {
            static ShaderManager instance;
            return instance;
        }

        static void setShaderDataLoad(std::string shaderName, std::string V_shaderPath, std::string F_shaderPath);
        static void LoadAllShaders();
        static Shader* Get(const std::string& name);

    private:

        struct ShaderDataSource {
            Shader::ShaderType shaderType;
            std::string sourcePath;
            std::string currentCode;

            // Constructor por defecto con valores predeterminados
            ShaderDataSource()
                : shaderType(Shader::ShaderType::NONE), sourcePath("") {}

            // Constructor con parámetros para inicializar miembros
            ShaderDataSource(Shader::ShaderType type, const std::string& path)
                : shaderType(type), sourcePath(path) {}
        };

        struct ShaderProgramSource {
            std::string name;

            ShaderDataSource vertexDataSource;
            ShaderDataSource fragmentDataSource;
            ShaderDataSource geometryDataSource;

            // Constructor con valores predeterminados que asigna los tipos de shaders adecuados
            ShaderProgramSource(const std::string& nameValue = "",
                const std::string& vertexPath = "",
                const std::string& fragmentPath = "",
                const std::string& geometryPath = "")
                : name(nameValue),
                // Asigna el tipo de shader correspondiente a cada ShaderDataSource
                vertexDataSource(Shader::ShaderType::VERTEX, vertexPath),
                fragmentDataSource(Shader::ShaderType::FRAGMENT, fragmentPath),
                geometryDataSource(Shader::ShaderType::GEOMETRY, geometryPath.empty() ? "" : geometryPath) // Asigna el path solo si geometryPath no está vacío
            {
            }
        };


        static void Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
        
        static void CleanUp();

        static const std::unordered_map<std::string, Shader*>& GetAllShaders()
        {
            return compiledShaders;
        }


        // Establece un tamaño grande para el buffer de código del shader.
        static const size_t ShaderBufferSize = 40000;

        static std::unordered_map<std::string, Shader*> compiledShaders;
        static std::vector<ShaderProgramSource> shaderProgramSources;
        static void ReloadAllShaders(ShaderProgramSource& programSource);

        static std::string readFile(const std::string& filePath);
    };
}

