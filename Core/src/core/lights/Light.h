#pragma once

#include "../Core.h"
#include "../model/Transform.h"


namespace libCore
{
    enum LightType {
        POINT,
        SPOT,
        AREA,
        DIRECTIONAL
    };

    class Light
    {
    public:
        int id = 0;
        LightType type;
        bool showDebug = false;

        libCore::Transform transform;

        glm::vec3 color;
        float intensity = 2.0f; // Añadido para controlar la intensidad de la luz

        //--POINT LIGHT--------------------------------------
        float Linear = 0.07f;
        float Quadratic = 0.01f;
        float Radius = 150.0f;
        float LightSmoothness = 2.8f;
        //---------------------------------------------------

        //--SPOT LIGHT---------------------------------------
        float innerCutoff = glm::cos(glm::radians(12.5f));
        float outerCutoff = glm::cos(glm::radians(17.5f));
        //---------------------------------------------------

        //--AREA LIGHT---------------------------------------
        std::vector<glm::vec3> areaLightpoints;
        bool twoSided = true;
        float LUT_SIZE = 64.0f;
        float LUT_SCALE = (64.0f - 1.0f) / 64.0f;
        float LUT_BIAS = 0.5f / 64.0f;
        //---------------------------------------------------


        //--DIRECTIONAL LIGHT--------------------------------
        glm::vec3 ambient = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

        glm::vec3 direction = glm::vec3(0.0f, -10.0f, 0.0f);
        float currentSceneRadius = 10.0f;
        float sceneRadiusOffset = 10.0f;

        // Shadow values
        int shadowMapResolution = 1024;

        glm::mat4 shadowMVP = glm::mat4(1.0f);
        bool drawShadows = false;
        float near_plane = 0.1f, far_plane = 100.0f;
        float shadowIntensity = 0.5f;
        bool usePoisonDisk = false;
        float orthoLeft = -10.0f;
        float orthoRight = 10.0f;
        float orthoBottom = -10.0f;
        float orthoTop = 10.0f;

        float orthoNear = 0.1f;
        float orthoNearOffset = 0.0f;
        float orthoFar = 100.0f;
        float orthoFarOffset = 0.0f;

        float angleX = 0.0f;
        float angleY = 0.0f;

        glm::mat4 shadowBias = glm::mat4(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
        );
        //--SCENE BOUNDS
        std::pair<glm::vec3, float> SceneBounds = { glm::vec3(0.0f), 30.0f };

        //---------------------------------------------------


        Light(LightType lightType) : type(lightType), color(1.0f, 1.0f, 1.0f), intensity(2.0f)
        {
            // Inicializa puntos con un cuadrado por defecto
            areaLightpoints = {
                glm::vec3(2.5f, 0.0f, 14.7f),
                glm::vec3(-2.5f, 0.0f, 14.7f),
                glm::vec3(-2.5f, 0.0f, -14.7f),
                glm::vec3(2.5f, 0.0f, -14.7f)
            };

            // DEBUG
            InitializeBuffers();
            UpdateVertices();
        }

        ~Light() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        void UpdateLightPosition();

        void SetLightDataToShader(const std::string& shader);
        


        //--DEBUG
        void DrawDebugLight(const std::string& shader);
        
        void UpdateVertices();
        


    private:
        //--DEBUG
        GLuint VAO, VBO;
        std::vector<glm::vec3> vertices;

        void InitializeBuffers();
        
    };
}