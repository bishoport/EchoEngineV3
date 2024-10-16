#pragma once

#include "../Core.h"

#include "DynamicSkybox.h"
#include "IBL.h"
#include "../../tools/Itc_matrix_floats.hpp"


#include "Viewport.h"

#include "../../managers/InputManager.h"
#include "../../managers/LightsManager.hpp"
#include "../../managers/EntityManager.h"


namespace libCore {

    class Renderer {

    public:
        //SKYBOX
        Scope<DynamicSkybox> dynamicSkybox = nullptr;

        //MISC
        bool m_wireframe = false;
        bool enableMultisample = true;

        //IBL
        Scope<IBL> ibl = nullptr;
        bool iblEnabled = true; // Variable para activar/desactivar IBL
        bool dynamicIBL = false;  // Cambia esto a `false` para IBL estático
        float iblIntensity = 0.0f;

        //SSAO
        bool  ssaoEnabled = true; // Variable para activar/desactivar SSAO
        float ssaoRadius = 0.12f;
        float ssaoBias = 0.3f;
        float ssaoIntensity = 1.8f;
        float ssaoPower = 1.8f;
        float F0Factor = 0.04f;

        //GLOBAL LIGHT
        float ambientLight = 1.0f;

        //HDR
        bool hdrEnabled = false;
        float hdrExposure = 1.0f;
        bool renderInPause = false;



        static Renderer& getInstance() {
            static Renderer instance;
            return instance;
        }

        void initialize();


        //--DEBUG
        void PushDebugGroup(const std::string& name) 
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.c_str());
        }
        void PopDebugGroup() 
        {
            glPopDebugGroup();
        }
        //--------------------------------------------------------------------------


        //--RENDERER
        void RenderViewport(const Ref<Viewport>& viewport, const Timestep& m_deltaTime);
        void ShowViewportInQuad(Ref<Viewport> viewport);
        //--------------------------------------------------------------------------


        //--GRID
        int gridSize = 10;
        glm::vec3 gridColor = glm::vec3(0.1f, 0.1f, 0.1f);
        float gridTransparency = 1.0f;
        float gridCellSize = 1.0f;
        float gridCellLineSize = 1.0f;

        //--AXIS
        glm::vec3 axisXColor = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 axisYColor = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 axisZColor = glm::vec3(0.0f, 0.0f, 1.0f);
        float axisSize = 0.1f; // Tamaño de las líneas del eje
        glm::vec3 axisOffset = glm::vec3(0.0f, 0.0f, 0.0f); // Offset para posicionar los ejes respecto a la cámara

    private:
        Renderer() {} // Constructor privado

        //--QUAD
        GLuint quadVAO, quadVBO;

        void setupQuad() {
            float quadVertices[] = {
                // positions  // texCoords
                -1.0f,  1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f, -1.0f, 1.0f, 0.0f,
                 1.0f, -1.0f, 1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,
                 1.0f,  1.0f, 1.0f, 1.0f
            };

            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glBindVertexArray(0);
        }
        void renderQuad() {
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
        //--------------------------------------------------------------------------



        void renderGrid()
        {
            static GLuint gridVAO = 0;
            static GLuint gridVBO = 0;

            // Si hay cambios en el tamaño del grid o el tamaño de las celdas, recrear el grid
            static int lastGridSize = gridSize;
            static float lastGridCellSize = gridCellSize;

            if (gridVAO == 0 || gridSize != lastGridSize || gridCellSize != lastGridCellSize)
            {
                lastGridSize = gridSize;
                lastGridCellSize = gridCellSize;

                // Generar las líneas para el grid
                std::vector<float> vertices;

                for (int i = -gridSize; i <= gridSize; ++i)
                {
                    float position = i * gridCellSize;

                    // Líneas en la dirección X (horizontales)
                    vertices.push_back(-gridSize * gridCellSize);
                    vertices.push_back(0.0f);
                    vertices.push_back(position);

                    vertices.push_back(gridSize * gridCellSize);
                    vertices.push_back(0.0f);
                    vertices.push_back(position);

                    // Líneas en la dirección Z (verticales)
                    vertices.push_back(position);
                    vertices.push_back(0.0f);
                    vertices.push_back(-gridSize * gridCellSize);

                    vertices.push_back(position);
                    vertices.push_back(0.0f);
                    vertices.push_back(gridSize * gridCellSize);
                }

                if (gridVAO == 0)
                {
                    glGenVertexArrays(1, &gridVAO);
                    glGenBuffers(1, &gridVBO);
                }

                glBindVertexArray(gridVAO);

                glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

                // Posiciones de vértices
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

                glBindVertexArray(0);
            }

            // Configurar las propiedades del shader antes de renderizar
            glLineWidth(gridCellLineSize);
            libCore::ShaderManager::Get("grid")->use();
            libCore::ShaderManager::Get("grid")->setVec3("gridColor", gridColor);
            libCore::ShaderManager::Get("grid")->setFloat("gridTransparency", gridTransparency);

            // Renderizar el grid
            glBindVertexArray(gridVAO);
            glDrawArrays(GL_LINES, 0, (gridSize * 2 + 1) * 4); // Cantidad de vértices en el grid
            glBindVertexArray(0);
        }
        //--------------------------------------------------------------------------


        //--WORLD_AXIS

        GLuint axesVAO;
        GLuint axesVBO;
        void setupAxes()
        {
            // Definir los vértices de las líneas de los ejes X, Y, Z
            float axesVertices[] = {
                // Eje X (rojo)
                0.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,

                // Eje Y (verde)
                0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,

                // Eje Z (azul)
                0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
            };

            glGenVertexArrays(1, &axesVAO);
            glGenBuffers(1, &axesVBO);

            glBindVertexArray(axesVAO);

            glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glBindVertexArray(0);
        }
        void renderAxes(const Ref<Viewport>& viewport)
        {
            glBindVertexArray(axesVAO);
            glLineWidth(2.0f); // Grosor de las líneas

            // Usa el shader para los ejes
            libCore::ShaderManager::Get("axes")->use();

            // Configura las matrices de vista, proyección y modelo
            glm::mat4 model = glm::mat4(1.0f);

            // Aplica la escala y el offset en la matriz de modelo para las líneas de los ejes
            model = glm::translate(model, viewport->camera->Position + axisOffset + viewport->camera->Orientation);
            model = glm::scale(model, glm::vec3(axisSize));

            libCore::ShaderManager::Get("axes")->setMat4("view", viewport->camera->view);
            libCore::ShaderManager::Get("axes")->setMat4("projection", viewport->camera->projection);
            libCore::ShaderManager::Get("axes")->setMat4("model", model);

            // Renderizar el eje X (rojo)
            libCore::ShaderManager::Get("axes")->setVec3("axisColor", axisXColor);
            glDrawArrays(GL_LINES, 0, 2);

            // Renderizar el eje Y (verde)
            libCore::ShaderManager::Get("axes")->setVec3("axisColor", axisYColor);
            glDrawArrays(GL_LINES, 2, 2);

            // Renderizar el eje Z (azul)
            libCore::ShaderManager::Get("axes")->setVec3("axisColor", axisZColor);
            glDrawArrays(GL_LINES, 4, 2);

            glBindVertexArray(0);

            // Ahora renderizamos las bolitas en los extremos
            glBindVertexArray(sphereVAO);
            glPointSize(10.0f); // Si estás usando GL_POINTS

            glm::mat4 sphereModel;

            // Eje X (rojo)
            libCore::ShaderManager::Get("axes")->setVec3("axisColor", axisXColor);
            sphereModel = glm::mat4(1.0f); // Partimos de una matriz identidad para las esferas
            sphereModel = glm::translate(sphereModel, viewport->camera->Position + axisOffset + viewport->camera->Orientation + glm::vec3(1.0f, 0.0f, 0.0f) * axisSize); // Mover al extremo del eje X
            libCore::ShaderManager::Get("axes")->setMat4("model", sphereModel);
            glDrawArrays(GL_POINTS, 0, 1);

            // Eje Y (verde)
            libCore::ShaderManager::Get("axes")->setVec3("axisColor", axisYColor);
            sphereModel = glm::mat4(1.0f); // Partimos de una matriz identidad para las esferas
            sphereModel = glm::translate(sphereModel, viewport->camera->Position + axisOffset + viewport->camera->Orientation + glm::vec3(0.0f, 1.0f, 0.0f) * axisSize); // Mover al extremo del eje Y
            libCore::ShaderManager::Get("axes")->setMat4("model", sphereModel);
            glDrawArrays(GL_POINTS, 0, 1);

            // Eje Z (azul)
            libCore::ShaderManager::Get("axes")->setVec3("axisColor", axisZColor);
            sphereModel = glm::mat4(1.0f); // Partimos de una matriz identidad para las esferas
            sphereModel = glm::translate(sphereModel, viewport->camera->Position + axisOffset + viewport->camera->Orientation + glm::vec3(0.0f, 0.0f, 1.0f) * axisSize); // Mover al extremo del eje Z
            libCore::ShaderManager::Get("axes")->setMat4("model", sphereModel);
            glDrawArrays(GL_POINTS, 0, 1);

            glBindVertexArray(0);
        }
        GLuint sphereVAO, sphereVBO;
        void setupSphere()
        {
            // Vertices para una esfera muy simple (puedes generar una esfera más compleja si lo prefieres)
            float sphereVertices[] = {
                // X, Y, Z
                0.0f, 0.0f, 0.0f, // Centro, usado como base para GL_POINTS o una esfera simple
                // Podrías agregar más vértices aquí para una esfera completa si lo prefieres
            };

            glGenVertexArrays(1, &sphereVAO);
            glGenBuffers(1, &sphereVBO);

            glBindVertexArray(sphereVAO);

            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glBindVertexArray(0);
        }
        //--------------------------------------------------------------------------


        //--SSAO       
        std::uniform_real_distribution<GLfloat> randomFloats{0.0f, 1.0f}; // Genera floats aleatorios entre 0.0 y 1.0
        std::default_random_engine generator;
        std::vector<glm::vec3> ssaoKernel;
        GLuint noiseTexture;
        float ourLerp(float a, float b, float f) {
            return a + f * (b - a);
        }
        std::vector<glm::vec3> generateSSAOKernel() {
            std::vector<glm::vec3> ssaoKernel;
            for (unsigned int i = 0; i < 64; ++i) {
                glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
                sample = glm::normalize(sample);
                sample *= randomFloats(generator);
                float scale = float(i) / 64.0f;

                // Escalar muestras para que estén más alineadas al centro del kernel
                scale = ourLerp(0.1f, 1.0f, scale * scale);
                sample *= scale;
                ssaoKernel.push_back(sample);
            }
            return ssaoKernel;
        }
        GLuint generateSSAONoiseTexture() {
            std::vector<glm::vec3> ssaoNoise;
            for (unsigned int i = 0; i < 16; i++) {
                glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // Rotar alrededor del eje z (en espacio tangente)
                ssaoNoise.push_back(noise);
            }
            unsigned int noiseTexture;
            glGenTextures(1, &noiseTexture);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            return noiseTexture;
        }
        //--------------------------------------------------------------------------


        //--LTC´s
        struct LTC_matrices {
            GLuint mat1;
            GLuint mat2;
        };
        LTC_matrices mLTC;
        GLuint loadMTexture()
        {
            GLuint texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_FLOAT, LTC1);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);
            return texture;
        }
        GLuint loadLUTTexture()
        {
            GLuint texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_FLOAT, LTC2);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);
            return texture;
        }
        //--------------------------------------------------------------------------



    };
}