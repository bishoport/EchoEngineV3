#pragma once

#include "../Core.h"

namespace libCore {

    class Shader
    {
    public:

        enum class ShaderType {
            NONE,
            VERTEX,
            FRAGMENT,
            GEOMETRY,
        };


        unsigned int ID;

        Shader() = default;


        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        Shader(const char* vertexPath, const char* fragmentPath = nullptr, const char* geometryPath = nullptr)
        {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::string geometryCode;

            try
            {
                // load shaders with includes
                vertexCode = readFile(vertexPath);
                fragmentCode = readFile(fragmentPath);

                // if geometry shader path is present, also load a geometry shader
                if (geometryPath != nullptr)
                {
                    geometryCode = readFile(geometryPath);
                }
            }
            catch (std::exception& e)
            {
                ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "SHADER::FILE_NOT_SUCCESFULLY_READ:" + std::string(e.what()));
            }

            const char* vShaderCode = vertexCode.c_str();

            // 2. compile shaders
            unsigned int vertex, fragment;


            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");


            // fragment Shader
            if (fragmentPath != nullptr) {
                const char* fShaderCode = fragmentCode.c_str();
                fragment = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fragment, 1, &fShaderCode, NULL);
                glCompileShader(fragment);
                checkCompileErrors(fragment, "FRAGMENT");
            }

            // if geometry shader is given, compile geometry shader
            unsigned int geometry;
            if (geometryPath != nullptr)
            {
                const char* gShaderCode = geometryCode.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }


            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            if (geometryPath != nullptr)
                glAttachShader(ID, geometry);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
            // delete the shaders as they're linked into our program now and no longer necessery
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (geometryPath != nullptr)
                glDeleteShader(geometry);

        }


        std::string processShaderIncludes(const std::string& source, const std::string& directory) {

            std::string content;
            std::istringstream sourceStream(source);  // Convierte el std::string en un stream
            std::string line;
            while (std::getline(sourceStream, line)) {  // Ahora puedes leer línea por línea
                // Check for #include directive
                if (line.find("#include") != std::string::npos) {
                    size_t startQuotePos = line.find("\"");
                    size_t endQuotePos = line.find("\"", startQuotePos + 1);
                    if (startQuotePos != std::string::npos && endQuotePos != std::string::npos) {
                        std::string includeFilename = line.substr(startQuotePos + 1, endQuotePos - startQuotePos - 1);
                        content.append(readFile("assets/shaders/" + includeFilename)); // Recursively read the included file
                    }
                    else {
                        std::cerr << "Error processing #include directive in " << std::endl;
                    }
                }
                else {
                    content.append(line + "\n");
                }
            }

            return content;
        }


        // constructor generates the shader on the fly from source strings
        Shader(bool primero, const std::string& vertexSource, const std::string& fragmentSource = "", const std::string& geometrySource = "")
        {
            // Convertir las cadenas de código fuente en punteros a char para su uso con OpenGL
            // Procesar los includes antes de compilar
            std::string processedVertexCode = processShaderIncludes(vertexSource, "/assets/shaders");
            std::string processedFragmentCode = processShaderIncludes(fragmentSource, "/assets/shaders");
            std::string processedGeometryCode = processShaderIncludes(geometrySource, "/assets/shaders");

            // Convertir las cadenas de código fuente en punteros a char para su uso con OpenGL
            const char* vShaderCode = processedVertexCode.c_str();
            const char* fShaderCode = processedFragmentCode.empty() ? nullptr : processedFragmentCode.c_str();
            const char* gShaderCode = processedGeometryCode.empty() ? nullptr : processedGeometryCode.c_str();

            //std::cout << fShaderCode << std::endl;

            // 2. compile shaders
            unsigned int vertex, fragment, geometry;

            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");


            // fragment Shader
            if (fragmentSource != "") {
                fragment = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fragment, 1, &fShaderCode, NULL);
                glCompileShader(fragment);
                checkCompileErrors(fragment, "FRAGMENT");
            }

            // if geometry shader is given, compile geometry shader

            if (geometrySource != "")
            {
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }


            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            if (geometrySource != "")
                glAttachShader(ID, geometry);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");


            // delete the shaders as they're linked into our program now and no longer necessery
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (geometrySource != "")
                glDeleteShader(geometry);
        }

        // activate the shader
        // ------------------------------------------------------------------------
        void use()
        {
            glUseProgram(ID);
        }
        // utility uniform functions
        // ------------------------------------------------------------------------
        void setBool(const std::string& name, bool value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        }
        // ------------------------------------------------------------------------
        void setInt(const std::string& name, int value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }
        // ------------------------------------------------------------------------
        void setFloat(const std::string& name, float value) const
        {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }
        // ------------------------------------------------------------------------
        void setVec2(const std::string& name, const glm::vec2& value) const
        {
            glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec2(const std::string& name, float x, float y) const
        {
            glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
        }
        // ------------------------------------------------------------------------
        void setVec3(const std::string& name, const glm::vec3& value) const
        {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec3(const std::string& name, float x, float y, float z) const
        {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
        }
        // ------------------------------------------------------------------------
        void setVec4(const std::string& name, const glm::vec4& value) const
        {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }
        void setVec4(const std::string& name, float x, float y, float z, float w)
        {
            glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
        }
        // ------------------------------------------------------------------------
        void setMat2(const std::string& name, const glm::mat2& mat) const
        {
            glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        // ------------------------------------------------------------------------
        void setMat3(const std::string& name, const glm::mat3& mat) const
        {
            glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        // ------------------------------------------------------------------------
        void setMat4(const std::string& name, const glm::mat4& mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }


    private:
        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        void checkCompileErrors(GLuint shader, std::string type)
        {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "SHADER_COMPILATION_ERROR of type: " + std::string(type));
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "SHADER_PROGRAM_LINKING_ERROR of type: " + std::string(type) + std::string(infoLog) );
                }
            }
        }




        std::string readFile(const std::string& filePath) {
            std::ifstream fileStream(filePath, std::ios::in);

            if (!fileStream.is_open()) {
                std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
                return "";
            }

            std::string content;
            std::string line = "";
            while (std::getline(fileStream, line)) {
                // Check for #include directive
                if (line.find("#include") != std::string::npos) {
                    size_t startQuotePos = line.find("\"");
                    size_t endQuotePos = line.find("\"", startQuotePos + 1);
                    if (startQuotePos != std::string::npos && endQuotePos != std::string::npos) {
                        std::string includeFilename = line.substr(startQuotePos + 1, endQuotePos - startQuotePos - 1);
                        content.append(readFile("assets/shaders/" + includeFilename)); // Recursively read the included file
                    }
                    else {
                        std::cerr << "Error processing #include directive in " << filePath << std::endl;
                    }
                }
                else {
                    content.append(line + "\n");
                }
            }

            fileStream.close();
            return content;
        }
    };
}