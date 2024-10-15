#pragma once

#include "../Core.h"

#include "DynamicSkybox.h"
#include "IBL.h"
#include "../../tools/Itc_matrix_floats.hpp"
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
        float ssaoRadius =0.12f;
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

        void initialize() 
        {
            setupAxes();
            setupSphere();

            //--SKYBOX
            std::vector<const char*> faces {
                "assets/Skybox/right.jpg",
                    "assets/Skybox/left.jpg",
                    "assets/Skybox/top.jpg",
                    "assets/Skybox/bottom.jpg",
                    "assets/Skybox/front.jpg",
                    "assets/Skybox/back.jpg"
            };
            dynamicSkybox = CreateScope<DynamicSkybox>(faces);
            //-------------------------------------------------------


            //--IBL
            ibl = CreateScope<IBL>();
            ibl->prepareIBL(640, 480,dynamicIBL);
            //-------------------------------------------------------


            //--SSAO
            setupQuad();// Configuración del quad para SSAO
            ssaoKernel = generateSSAOKernel();         // Genera el kernel
            noiseTexture = generateSSAONoiseTexture(); // Genera textura de ruido SSAO
            //-------------------------------------------------------


            //--AREA LIGHT
            //mLTC.mat1 = LUTGenerator::GetInstance().createLTC1Texture(64);
            //mLTC.mat2 = LUTGenerator::GetInstance().createLTC2Texture(64);
            mLTC.mat1 = loadMTexture();
            mLTC.mat2 = loadLUTTexture();
            //-------------------------------------------------------
        }


        //--DEBUG
        void PushDebugGroup(const std::string& name) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.c_str());
        }
        void PopDebugGroup() {
            glPopDebugGroup();
        }
        //--------------------------------------------------------------------------


        //--RENDERER
        void RenderViewport(const Ref<Viewport>& viewport, const Timestep& m_deltaTime) 
        { 
            if (renderInPause == true) return;


            if (enableMultisample)
            {
                glEnable(GL_MULTISAMPLE);
            }
            else 
            {
                glDisable(GL_MULTISAMPLE);
            }

            //// Limpiar el buffer de color y profundidad del framebuffer por defecto
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Actualizar la cámara
            viewport->camera->Inputs(m_deltaTime);
            viewport->camera->updateMatrix();

            glEnable(GL_DEPTH_TEST); // Habilitar el test de profundidad





            //---------------------------------------------------------------------------------
            //-----------------------COLOR PICKING PASS (SOLO EN MODO EDITOR)------------------
            //---------------------------------------------------------------------------------
            PushDebugGroup("Color Picking Pass");
            if (viewport->framebuffer_picking != nullptr)
            {
                viewport->framebuffer_picking->bindFBO();
                glViewport(0, 0, static_cast<GLsizei>(viewport->viewportSize.x), static_cast<GLsizei>(viewport->viewportSize.y));
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar color y profundidad

                // Usar el shader de picking
                libCore::ShaderManager::Get("picking")->use();

                // Pasar las matrices de proyección y vista
                libCore::ShaderManager::Get("picking")->setMat4("projection", viewport->camera->projection);
                libCore::ShaderManager::Get("picking")->setMat4("view", viewport->camera->view);

                // Dibujar los objetos con colores únicos
                EntityManager::GetInstance().DrawGameObjects("picking");

                viewport->framebuffer_picking->unbindFBO();
            }
            PopDebugGroup();
            //--------------------------------------------------------------------------------
            //--------------------------------------------------------------------------------











            //--------------------------------------------------------------------------------
            //-----------------------DIRECTIONAL LIGHT SHADOW PASS----------------------------
            //--------------------------------------------------------------------------------
            if (viewport->framebuffer_shadowmap != nullptr)
            {
                auto& sunLight = dynamicSkybox->sunLight;
                if (sunLight != nullptr && sunLight->drawShadows) {
                    PushDebugGroup("Directional Light Shadow Pass");

                    // Configura la vista y proyección desde la perspectiva de la luz
                    glm::mat4 shadowProjMat = glm::ortho(sunLight->orthoLeft,
                        sunLight->orthoRight,
                        sunLight->orthoBottom,
                        sunLight->orthoTop,
                        sunLight->orthoNear,
                        sunLight->orthoFar);

                    glm::mat4 shadowViewMat = glm::lookAt(sunLight->transform->position,
                        sunLight->transform->position + sunLight->direction,
                        glm::vec3(0, 1, 0));

                    sunLight->shadowMVP = shadowProjMat * shadowViewMat;

                    // Bind the shadow framebuffer
                    viewport->framebuffer_shadowmap->bindFBO();
                    glViewport(0, 0, static_cast<GLsizei>(viewport->viewportSize.x), static_cast<GLsizei>(viewport->viewportSize.y));
                    glClear(GL_DEPTH_BUFFER_BIT);

                    libCore::ShaderManager::Get("direct_light_depth_shadows")->use();
                    libCore::ShaderManager::Get("direct_light_depth_shadows")->setMat4("shadowMVP", sunLight->shadowMVP);

                    // Draw the models in the scene
                    EntityManager::GetInstance().DrawGameObjects("direct_light_depth_shadows");

                    viewport->framebuffer_shadowmap->unbindFBO();
                    glFinish(); // Espera a que el framebuffer de sombra termine antes de proceder
                    PopDebugGroup();
                }
                //glViewport(0, 0, static_cast<GLsizei>(viewport->viewportSize.x), static_cast<GLsizei>(viewport->viewportSize.y));
                int shadowMapWidth = 1024; // o el tamaño que definas
                int shadowMapHeight = 1024;
                glViewport(0, 0, shadowMapWidth, shadowMapHeight);
            }
            //--------------------------------------------------------------------------------
            //--------------------------------------------------------------------------------



            //------------------------------------------------------------------------------------------------------
            //---------------------------------------------DEFERRED-------------------------------------------------
            //------------------------------------------------------------------------------------------------------
            if (viewport->gBuffer != nullptr)
            {
                // 1.1 Renderizado de geometría en el GBuffer
                PushDebugGroup("Deferred Geometry Pass");
                viewport->gBuffer->bindGBuffer();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glViewport(0, 0, static_cast<GLsizei>(viewport->viewportSize.x), static_cast<GLsizei>(viewport->viewportSize.y));
                // Usar el shader de geometría
                libCore::ShaderManager::Get("geometryPass")->use();
                libCore::ShaderManager::Get("geometryPass")->setMat4("projection", viewport->camera->projection);
                libCore::ShaderManager::Get("geometryPass")->setMat4("view", viewport->camera->view);

                // Configurar el modo de polígono para wireframe o sólido
                if (m_wireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                EntityManager::GetInstance().DrawGameObjects("geometryPass");

                // Desvinculamos el GBuffer
                viewport->gBuffer->unbindGBuffer();
                glFinish(); // Asegura que el GBuffer esté completamente procesado
                PopDebugGroup();
            }
            //--------------------------------------------------------------------------------
            //--------------------------------------------------------------------------------



            ////--------------------------------------------------------------------------------
            ////-----------------------SSAO-----------------------------------------------------
            ////--------------------------------------------------------------------------------
            if (viewport->framebuffer_SSAO != nullptr && ssaoEnabled)
            {
                // Renderizado SSAO
                PushDebugGroup("SSAO Pass");

                //Copiar el buffer de profundidad del GBuffer al FBO SSAO
                viewport->blitGBO2FBO(viewport->gBuffer, viewport->framebuffer_SSAO, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                glFinish(); // Asegura que el blit haya completado antes de proceder con SSAO

                viewport->framebuffer_SSAO->bindFBO();
                glClear(GL_COLOR_BUFFER_BIT); // No se limpia aquí el GL_DEPTH_BUFFER_BIT!!!!
                libCore::ShaderManager::Get("ssao")->use();

                for (unsigned int i = 0; i < 64; ++i)
                    libCore::ShaderManager::Get("ssao")->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

                libCore::ShaderManager::Get("ssao")->setMat4("projection", viewport->camera->projection);

                libCore::ShaderManager::Get("ssao")->setInt("gPosition", 0);
                libCore::ShaderManager::Get("ssao")->setInt("gPositionView", 1);
                libCore::ShaderManager::Get("ssao")->setInt("gNormal", 2);
                libCore::ShaderManager::Get("ssao")->setInt("texNoise", 3);

                // Establecer valores de radio, sesgo (bias), intensidad y power para SSAO
                libCore::ShaderManager::Get("ssao")->setFloat("radius", ssaoRadius);
                libCore::ShaderManager::Get("ssao")->setFloat("bias", ssaoBias);
                libCore::ShaderManager::Get("ssao")->setFloat("intensity", ssaoIntensity);
                libCore::ShaderManager::Get("ssao")->setFloat("power", ssaoPower);

                libCore::ShaderManager::Get("ssao")->setFloat("screenWidth", viewport->viewportSize.x);
                libCore::ShaderManager::Get("ssao")->setFloat("screenHeight", viewport->viewportSize.y);

                viewport->gBuffer->bindTexture("position", 0);
                viewport->gBuffer->bindTexture("positionView", 1);
                viewport->gBuffer->bindTexture("normal", 2);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, noiseTexture);

                renderQuad();
                viewport->framebuffer_SSAO->unbindFBO();
                PopDebugGroup();
                //--------------------------------------------------------------------------------------------------------------

                //---Renderizado SSAO_BLUR
                PushDebugGroup("SSAO Blur Pass");
                
                //Copiar el buffer de profundidad del GBuffer al FBO SSAOBlur
                viewport->blitGBO2FBO(viewport->gBuffer, viewport->framebuffer_SSAOBlur, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

                viewport->framebuffer_SSAOBlur->bindFBO();
                glClear(GL_COLOR_BUFFER_BIT);
                libCore::ShaderManager::Get("ssaoBlur")->use();
                libCore::ShaderManager::Get("ssaoBlur")->setInt("ssaoInput", 0);
                viewport->framebuffer_SSAO->bindTexture("color", 0);
                renderQuad();
                viewport->framebuffer_SSAOBlur->unbindFBO();
                PopDebugGroup();
                //--------------------------------------------------------------------------------
            }
            //--------------------------------------------------------------------------------
            //--------------------------------------------------------------------------------





            //--------------------------------------------------------------------------------
            //-----------------------LIGHTING PASS--------------------------------------------
            //--------------------------------------------------------------------------------
            if (viewport->gBuffer != nullptr && viewport->framebuffer_SSAO != nullptr)
            {
                PushDebugGroup("Lighting Pass");
                viewport->framebuffer_deferred->bindFBO();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // forzamos el quitar el wireframe para mostrar el quad

                // Pasamos las texturas generadas por el GBuffer-> Shader de iluminación para usar esta información y combinar las luces
                libCore::ShaderManager::Get("lightingPass")->use();
                libCore::ShaderManager::Get("lightingPass")->setInt("gPosition", 0);
                libCore::ShaderManager::Get("lightingPass")->setInt("gNormal", 1);
                libCore::ShaderManager::Get("lightingPass")->setInt("gAlbedo", 2);
                libCore::ShaderManager::Get("lightingPass")->setInt("gMetallicRoughness", 3);
                libCore::ShaderManager::Get("lightingPass")->setInt("gSpecular", 4);
                libCore::ShaderManager::Get("lightingPass")->setInt("ssaoTexture", 5);
                viewport->gBuffer->bindTexture("position", 0);
                viewport->gBuffer->bindTexture("normal", 1);
                viewport->gBuffer->bindTexture("albedo", 2);
                viewport->gBuffer->bindTexture("metallicRoughness", 3);
                viewport->gBuffer->bindTexture("specular", 4);
                viewport->framebuffer_SSAO->bindTexture("color", 5);

                //ALL Lights
                //LightsManager::GetInstance().SetLightDataInShader("lightingPass");
                auto& sunLight = dynamicSkybox->sunLight;
                sunLight->SetLightDataToShader("lightingPass");

                //-DIRECTIONAL Light
                libCore::ShaderManager::Get("lightingPass")->setMat4("lightSpaceMatrix", sunLight->shadowMVP);
                libCore::ShaderManager::Get("lightingPass")->setBool("useSSAO", ssaoEnabled);
                libCore::ShaderManager::Get("lightingPass")->setFloat("exposure", hdrExposure);
                libCore::ShaderManager::Get("lightingPass")->setFloat("ambientLight", ambientLight);
                libCore::ShaderManager::Get("lightingPass")->setVec3("viewPos", viewport->camera->Position);
                libCore::ShaderManager::Get("lightingPass")->setFloat("F0Factor", F0Factor);


                //-AREA LIGHT
                libCore::ShaderManager::Get("lightingPass")->setInt("LTC1", 6);
                libCore::ShaderManager::Get("lightingPass")->setInt("LTC2", 7);
                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_2D, mLTC.mat1);
                glActiveTexture(GL_TEXTURE7);
                glBindTexture(GL_TEXTURE_2D, mLTC.mat2);
                //----------------------------------------------------------------------------------------------


                //-IBL
                libCore::ShaderManager::Get("lightingPass")->setBool("useIBL", iblEnabled);
                libCore::ShaderManager::Get("lightingPass")->setFloat("iblIntensity", iblIntensity);
                libCore::ShaderManager::Get("lightingPass")->setInt("irradianceMap", 8);
                libCore::ShaderManager::Get("lightingPass")->setInt("prefilterMap", 9);
                libCore::ShaderManager::Get("lightingPass")->setInt("brdfLUT", 10);

                glActiveTexture(GL_TEXTURE8);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->irradianceMap);
                glActiveTexture(GL_TEXTURE9);
                glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->prefilterMap);
                glActiveTexture(GL_TEXTURE10);
                glBindTexture(GL_TEXTURE_2D, ibl->brdfLUTTexture);
                //----------------------------------------------------------------------------------------------



                // SHADOW MAP
                if (sunLight->drawShadows)
                {
                    libCore::ShaderManager::Get("lightingPass")->setBool("useShadows", sunLight->drawShadows); // Activa las sombras
                    libCore::ShaderManager::Get("lightingPass")->setInt("shadowMap", 11);

                    // Utiliza el método bindTexture del FBO
                    viewport->framebuffer_shadowmap->bindTexture("depth", 11);
                }
                renderQuad();

                // Desvincula el FBO deferred
                viewport->framebuffer_deferred->unbindFBO();
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // Desvincular FBO
                PopDebugGroup();
            }
            //--------------------------------------------------------------------------------
            //--------------------------------------------------------------------------------




            //------------------------------------------------------------------------------------------------------
            //---------------------------------------------FORWARD--------------------------------------------------
            //------------------------------------------------------------------------------------------------------
            if (viewport->framebuffer_forward != nullptr)
            {
                PushDebugGroup("Forward Pass");

                viewport->blitGBO2FBO(viewport->gBuffer, viewport->framebuffer_forward, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

                viewport->framebuffer_forward->bindFBO();
                glClear(GL_COLOR_BUFFER_BIT); // No se limpia aquí el GL_DEPTH_BUFFER_BIT!!!!
                glDepthFunc(GL_LESS);
                //------------------------------------------------------------------------------------------


                // PASADA SKYBOX
                dynamicSkybox->Render(viewport->camera->view, viewport->camera->projection);
                //------------------------------------------------------------------------------------------

                // PASADA DE DEBUG
                libCore::ShaderManager::Get("debug")->use();
                libCore::ShaderManager::Get("debug")->setMat4("view", viewport->camera->view);
                libCore::ShaderManager::Get("debug")->setMat4("projection", viewport->camera->projection);
                //------------------------------------------------------------------------------------------

                //DEBUG de Luces
                LightsManager::GetInstance().DrawDebugLights("debug");
                //------------------------------------------------------------------------------------------

                //DEBUG AABB
                EntityManager::GetInstance().DrawABBGameObjectMeshComponent("debug");
                //------------------------------------------------------------------------------------------



                //GRID
                glEnable(GL_BLEND);//Empieza zona alpha
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                // Asegúrate de que la cámara esté viendo el grid correctamente
                libCore::ShaderManager::Get("grid")->use();
                libCore::ShaderManager::Get("grid")->setMat4("view", viewport->camera->view);
                libCore::ShaderManager::Get("grid")->setMat4("projection", viewport->camera->projection);

                // Opcionalmente, podrías setear la matriz del modelo
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                libCore::ShaderManager::Get("grid")->setMat4("model", modelMatrix);

                // Opcionalmente, deshabilita el depth test para asegurarte de que se renderice en la parte superior
                glDisable(GL_DEPTH_TEST);

                // Renderizar los ejes en el origen
                renderAxes(viewport);

                //Render camera Frustrum
                //viewport->camera->RenderFrustum();
                //viewport->gameCamera->RenderFrustum();

                // Renderiza el grid
                renderGrid();

                // Habilita nuevamente el depth test para el resto de la escena
                glEnable(GL_DEPTH_TEST);
                //------------------------------------------------------------------------------------------

                // PASADA DE TEXTOS
                glm::mat4 model = glm::mat4(1.0f);
                libCore::ShaderManager::Get("text")->use();
                libCore::ShaderManager::Get("text")->setMat4("projection", viewport->camera->projection);
                libCore::ShaderManager::Get("text")->setMat4("model", model);
                libCore::ShaderManager::Get("text")->setMat4("view", viewport->camera->view);

                //--> Render Textos AQUI!!!
                glDisable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ZERO);//Acaba zona alpha
                //------------------------------------------------------------------------------------------
                
                // Desligar el FBO forward
                viewport->framebuffer_forward->unbindFBO();
                PopDebugGroup();
            }
            //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------------------------



            //------------------------------------------------------------------------------------------------------
            //------------COMBINE DEFERRED+FORWARD+SSAO+SHADOWS-----------------------------------------------------
            //------------------------------------------------------------------------------------------------------
            if (viewport->framebuffer_final != nullptr)
            {
                PushDebugGroup("Combine Pass");
                viewport->framebuffer_final->bindFBO();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                libCore::ShaderManager::Get("combinePass")->use();
                viewport->framebuffer_deferred->bindTexture("color", 0);
                viewport->framebuffer_forward->bindTexture("color", 1);
                viewport->framebuffer_SSAOBlur->bindTexture("color", 2);
                viewport->framebuffer_shadowmap->bindTexture("depth", 3);

                auto& sunLight = dynamicSkybox->sunLight;
                if (sunLight->drawShadows)
                {
                    viewport->framebuffer_shadowmap->bindTexture("depth", 3);
                }

                libCore::ShaderManager::Get("combinePass")->setInt("deferredTexture", 0);
                libCore::ShaderManager::Get("combinePass")->setInt("forwardTexture", 1);
                libCore::ShaderManager::Get("combinePass")->setInt("ssaoTexture", 2);
                libCore::ShaderManager::Get("combinePass")->setInt("shadowTexture", 3);
                libCore::ShaderManager::Get("combinePass")->setBool("useSSAO", ssaoEnabled);
                libCore::ShaderManager::Get("combinePass")->setBool("useShadow", sunLight->drawShadows);
                renderQuad();
                PopDebugGroup();
            }
            //------------------------------------------------------------------------------------------
            //------------------------------------------------------------------------------------------

            viewport->UnbindAll();
        }

        void ShowViewportInQuad(Ref<Viewport> viewport) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            libCore::ShaderManager::Get("colorQuadFBO")->use();
            viewport->framebuffer_final->bindTexture("color", 0);
            libCore::ShaderManager::Get("colorQuadFBO")->setInt("screenTexture", 0);
            renderQuad();
        }
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