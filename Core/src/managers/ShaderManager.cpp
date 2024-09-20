#include "ShaderManager.h"

namespace fs = std::filesystem;

namespace libCore
{
    std::vector<ShaderManager::ShaderProgramSource> ShaderManager::shaderProgramSources;
    std::unordered_map<std::string, Shader*> ShaderManager::compiledShaders;


    void ShaderManager::setShaderDataLoad(std::string shaderName, std::string V_shaderPath, std::string F_shaderPath)
    {
        //--LOAD SHADERS
        ShaderManager::shaderProgramSources.emplace_back(shaderName, V_shaderPath, F_shaderPath);
    }


    void ShaderManager::LoadAllShaders()
    {

        //CARGAMOS LOS DATOS
        for (const auto& shaderSource : shaderProgramSources) {

            if (shaderSource.geometryDataSource.sourcePath.c_str() == "")
            {
                ShaderManager::Load(shaderSource.name.c_str(), shaderSource.vertexDataSource.sourcePath.c_str(), shaderSource.fragmentDataSource.sourcePath.c_str());
            }
            else
            {
                ShaderManager::Load(shaderSource.name.c_str(), shaderSource.vertexDataSource.sourcePath.c_str(), shaderSource.fragmentDataSource.sourcePath.c_str(), shaderSource.geometryDataSource.sourcePath.c_str());
            }
        }
        //----------------------------------------------------------------------------------------------------------------------------
    }


    void ShaderManager::Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
        Shader* shader = new Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.empty() ? nullptr : geometryPath.c_str());
        compiledShaders[name] = shader;
    }

    Shader* ShaderManager::Get(const std::string& name) {
        if (compiledShaders.find(name) != compiledShaders.end()) {
            return compiledShaders[name];
        }
        return nullptr;
    }

    void ShaderManager::CleanUp() {
        for (auto& [name, shader] : compiledShaders) {
            delete shader;
        }
        compiledShaders.clear();
    }

    void ShaderManager::ReloadAllShaders(ShaderProgramSource& programSource) {
        // Obtén el programa de shader actual y desvincula todos los shaders.
        glUseProgram(0);
        GLuint programID = compiledShaders[programSource.name]->ID;
        GLint shaderCount;
        glGetProgramiv(programID, GL_ATTACHED_SHADERS, &shaderCount);

        std::vector<GLuint> attachedShaders(shaderCount);
        glGetAttachedShaders(programID, shaderCount, NULL, &attachedShaders[0]);

        for (GLuint shader : attachedShaders) {
            glDetachShader(programID, shader);
            glDeleteShader(shader);
        }

        for (const auto& shaderSource : shaderProgramSources) {

            if (shaderSource.geometryDataSource.sourcePath.c_str() == "")
            {
                Shader* shader = new Shader(shaderSource.vertexDataSource.currentCode.c_str(), shaderSource.fragmentDataSource.currentCode.c_str());
                compiledShaders[shaderSource.name] = shader;
            }
            else
            {
                Shader* shader = new Shader(true,shaderSource.vertexDataSource.currentCode.c_str(), 
                                            shaderSource.fragmentDataSource.currentCode.c_str(), 
                                            shaderSource.geometryDataSource.currentCode.c_str());
                compiledShaders[shaderSource.name] = shader;
            }
        }
    }

    std::vector<std::string> GetShaderFiles(const std::string& path) {
        std::vector<std::string> items;

        for (const auto& entry : fs::directory_iterator(path)) {
            items.push_back(entry.path().filename().string());
        }
        return items;
    }

    std::string ShaderManager::readFile(const std::string& filePath) {

        std::ifstream fileStream(filePath, std::ios::in);

        if (!fileStream.is_open()) {
            std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
            return "";
        }

        std::string content;
        std::string line = "";
        while (!fileStream.eof()) {
            std::getline(fileStream, line);
            content.append(line + "\n");
        }

        fileStream.close();
        return content;
    }
}

