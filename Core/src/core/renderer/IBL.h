#pragma once

#include "../Core.h"
#include "../../managers/ModelLoader.h"

namespace libCore
{

	class IBL {

    public:

        //IBL
        GLuint IBL_FBO = 0;
        GLuint IBL_RBO = 0;

        GLuint envCubemap = 0;
        GLuint irradianceMap = 0;
        GLuint prefilterMap = 0;
        GLuint brdfLUTTexture = 0;


        void prepareIBL(int screenWidth, int screenHeight, bool dynamic);

        void UpdateIBL(const glm::vec3& cameraPosition, const std::vector<Ref<libCore::Model>>& modelsInScene, float nearPlane, float farPlane);
        //----------------------------------------------------

        GLuint m_hdrTexture;

        // Variables para manipulación desde ImGUI
        float ambientStrength = 0.1f;
        float specularStrength = 0.5f;
        float shininess = 32.0f;

       

    private:

        int  m_screenWidth = 0;
        int  m_screenHeight = 0;
        bool m_dynamic = false;

        void configureStaticIBL();
        void configureDynamicIBL();

        GLuint cubeVAO = 0;
        GLuint cubeVBO = 0;
        GLuint quadVAO = 0;
        GLuint quadVBO = 0;

        void renderCube();
        void renderQuad();
    };
}