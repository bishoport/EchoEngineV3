#pragma once
#include "../Core.h"
#include "../../managers/ShaderManager.h"
#include "../../managers/TextureManager.h"
#include "../lights/SunLight.h"

namespace libCore {

    class DynamicSkybox {
    public:

        // Propiedad del SunLight
        Ref<SunLight> sunLight;

        bool useTexture = false;

        glm::vec3 sunPosition = glm::vec3(0.0f, 0.0f, 1.0f);
        float m_gradientIntensity = 1.0f;
        glm::vec3 m_sunDiskSize = glm::vec3(0.04f, 0.04f, 0.04f);

        float auraSize = 0.02f;
        float auraIntensity = 0.5f;
        float edgeSoftness = 0.5f;

        glm::vec3 dayLightColor = glm::vec3(73.0f / 255.0f, 132.0f / 255.0f, 182.0f / 255.0f);
        glm::vec3 sunsetColor = glm::vec3(210.0f / 255.0f, 117.0f / 255.0f, 41.0f / 255.0f);
        glm::vec3 dayNightColor = glm::vec3(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);
        glm::vec3 groundColor = glm::vec3(36.0f / 255.0f, 89.0f / 255.0f, 122.0f / 255.0f);

        float lowerBound = -0.05f;
        float upperBound = 0.05f;

        // Stars
        float starDensity = 0.0005f; // Densidad de estrellas
        float starSizeMin = 0.5f; // Tamaño mínimo de las estrellas
        float starSizeMax = 1.5f; // Tamaño máximo de las estrellas
        float starBrightnessMin = 0.5f; // Brillo mínimo de las estrellas
        float starBrightnessMax = 1.0f; // Brillo máximo de las estrellas
        glm::vec2 starCoordScale = glm::vec2(100.0f, 100.0f); // Escala de las coordenadas para las estrellas




        DynamicSkybox(const std::vector<const char*> faces)
        {
            sunLight = CreateRef<SunLight>();  // Inicializa el sol
            sunLight->transform = CreateRef<Transform>();  // Asignar el transform

            // cube VAO
            glGenVertexArrays(1, &cubeVAO);
            glGenBuffers(1, &cubeVBO);
            glBindVertexArray(cubeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

            // skybox VAO
            glGenVertexArrays(1, &skyboxVAO);
            glGenBuffers(1, &skyboxVBO);
            glBindVertexArray(skyboxVAO);
            glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


            cubemapTexture = TextureManager::getInstance().LoadImagesForCubemap(faces);

        }

        ~DynamicSkybox() {
            glDeleteVertexArrays(1, &skyboxVAO);
            glDeleteBuffers(1, &skyboxVBO);
        }


        // Sincronizar la posición del SunLight con el Skybox
        void SyncSunLightWithSkybox()
        {
            // Sincroniza la posición del SunLight con la posición del sol en el Skybox
            sunLight->transform->position = sunPosition;

            // Asegúrate de que la luz mire hacia el centro de la escena (0, 0, 0)
            sunLight->direction = glm::normalize(-sunLight->transform->position);

            // Actualizar los ángulos (angleX y angleY) para que coincidan con la posición del Sol en el Skybox
            float distance = glm::length(sunPosition);
            sunLight->angleX = atan2(sunPosition.z, sunPosition.x);
            sunLight->angleY = acos(sunPosition.y / distance);

        }

        void Render(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
        {
            // draw skybox as last
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            libCore::ShaderManager::Get("dynamicSkybox")->use();

            // Elimina la traslación de la matriz de vista
            viewMatrix = glm::mat4(glm::mat3(viewMatrix)); // Elimina la traslación, manteniendo solo la rotación

            // Escala la matriz de vista para hacer el skybox más grande
            float scale = 1.0f; // Ajusta este valor para obtener el tamaño deseado para tu skybox
            //viewMatrix = glm::scale(viewMatrix, glm::vec3(scale, scale, scale));

            libCore::ShaderManager::Get("dynamicSkybox")->setMat4("view", viewMatrix);
            libCore::ShaderManager::Get("dynamicSkybox")->setMat4("projection", projectionMatrix);

            libCore::ShaderManager::Get("dynamicSkybox")->setVec3("sunPosition", sunPosition);
            libCore::ShaderManager::Get("dynamicSkybox")->setVec3("sunDiskSize", m_sunDiskSize);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("gradientIntensity", m_gradientIntensity);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("uAuraSize", auraSize);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("uAuraIntensity", auraIntensity);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("uEdgeSoftness", edgeSoftness);
            libCore::ShaderManager::Get("dynamicSkybox")->setVec3("uDayLightColor", dayLightColor);
            libCore::ShaderManager::Get("dynamicSkybox")->setVec3("uSunsetColor", sunsetColor);
            libCore::ShaderManager::Get("dynamicSkybox")->setVec3("uDayNightColor", dayNightColor);
            libCore::ShaderManager::Get("dynamicSkybox")->setVec3("uGroundColor", groundColor);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("uUpperBound", upperBound);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("uLowerBound", lowerBound);
            libCore::ShaderManager::Get("dynamicSkybox")->setBool("useSkyboxTexture", useTexture);

            // Set star parameters
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("starDensity", starDensity);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("starSizeMin", starSizeMin);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("starSizeMax", starSizeMax);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("starBrightnessMin", starBrightnessMin);
            libCore::ShaderManager::Get("dynamicSkybox")->setFloat("starBrightnessMax", starBrightnessMax);
            libCore::ShaderManager::Get("dynamicSkybox")->setVec2("starCoordScale", starCoordScale);


            // skybox cube
            //glDepthMask(GL_FALSE);
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


    private:
        unsigned int cubeVAO, cubeVBO;
        GLuint skyboxVAO, skyboxVBO;
        glm::vec3 m_SunPosition;

        GLuint cubemapTexture;

        float cubeSize = 500.0f;
        
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float cubeVertices[180] = {
            // positions               // texture Coords
            -cubeSize, -cubeSize, -cubeSize,       0.0f, 0.0f,
             cubeSize, -cubeSize, -cubeSize,       1.0f, 0.0f,
             cubeSize,  cubeSize, -cubeSize,       1.0f, 1.0f,
             cubeSize,  cubeSize, -cubeSize,       1.0f, 1.0f,
            -cubeSize,  cubeSize, -cubeSize,       0.0f, 1.0f,
            -cubeSize, -cubeSize, -cubeSize,       0.0f, 0.0f,

            -cubeSize, -cubeSize,  cubeSize,       0.0f, 0.0f,
             cubeSize, -cubeSize,  cubeSize,       1.0f, 0.0f,
             cubeSize,  cubeSize,  cubeSize,       1.0f, 1.0f,
             cubeSize,  cubeSize,  cubeSize,       1.0f, 1.0f,
            -cubeSize,  cubeSize,  cubeSize,       0.0f, 1.0f,
            -cubeSize, -cubeSize,  cubeSize,       0.0f, 0.0f,

            -cubeSize,  cubeSize,  cubeSize,       1.0f, 0.0f,
            -cubeSize,  cubeSize, -cubeSize,       1.0f, 1.0f,
            -cubeSize, -cubeSize, -cubeSize,       0.0f, 1.0f,
            -cubeSize, -cubeSize, -cubeSize,       0.0f, 1.0f,
            -cubeSize, -cubeSize,  cubeSize,       0.0f, 0.0f,
            -cubeSize,  cubeSize,  cubeSize,       1.0f, 0.0f,

             cubeSize,  cubeSize,  cubeSize,       1.0f, 0.0f,
             cubeSize,  cubeSize, -cubeSize,       1.0f, 1.0f,
             cubeSize, -cubeSize, -cubeSize,       0.0f, 1.0f,
             cubeSize, -cubeSize, -cubeSize,       0.0f, 1.0f,
             cubeSize, -cubeSize,  cubeSize,       0.0f, 0.0f,
             cubeSize,  cubeSize,  cubeSize,       1.0f, 0.0f,

            -cubeSize, -cubeSize, -cubeSize,       0.0f, 1.0f,
             cubeSize, -cubeSize, -cubeSize,       1.0f, 1.0f,
             cubeSize, -cubeSize,  cubeSize,       1.0f, 0.0f,
             cubeSize, -cubeSize,  cubeSize,       1.0f, 0.0f,
            -cubeSize, -cubeSize,  cubeSize,       0.0f, 0.0f,
            -cubeSize, -cubeSize, -cubeSize,       0.0f, 1.0f,

            -cubeSize,  cubeSize, -cubeSize,       0.0f, 1.0f,
             cubeSize,  cubeSize, -cubeSize,       1.0f, 1.0f,
             cubeSize,  cubeSize,  cubeSize,       1.0f, 0.0f,
             cubeSize,  cubeSize,  cubeSize,       1.0f, 0.0f,
            -cubeSize,  cubeSize,  cubeSize,       0.0f, 0.0f,
            -cubeSize,  cubeSize, -cubeSize,       0.0f, 1.0f
        };
        float skyboxVertices[108] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
    };
}
