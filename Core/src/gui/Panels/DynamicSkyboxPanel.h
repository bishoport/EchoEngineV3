#pragma once
#include "PanelBase.h"
#include "../../core/renderer/Renderer.hpp"

namespace libCore
{
    class DynamicSkyboxPanel : public PanelBase
    {
    public:
        DynamicSkyboxPanel() : PanelBase("Skybox") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            // Dynamic Skybox Section
            ImGui::Text(ICON_FA_CLOUD_SUN " Dynamic Skybox");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Checkbox(ICON_FA_CHECK_CIRCLE " Use Skybox Texture", &Renderer::getInstance().dynamicSkybox->useTexture);
            ImGui::ColorEdit3(ICON_FA_TINT " Day Light Color", (float*)&Renderer::getInstance().dynamicSkybox->dayLightColor);
            ImGui::ColorEdit3(ICON_FA_TINT " Sunset Color", (float*)&Renderer::getInstance().dynamicSkybox->sunsetColor);
            ImGui::ColorEdit3(ICON_FA_TINT " Day Night Color", (float*)&Renderer::getInstance().dynamicSkybox->dayNightColor);
            ImGui::ColorEdit3(ICON_FA_TINT " Ground Color", (float*)&Renderer::getInstance().dynamicSkybox->groundColor);
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            // Sun Section
            if (ImGui::CollapsingHeader(ICON_FA_SUN " Sun"))
            {
                static float orbitRadius = 50.0f;  // Radio de la órbita del sol
                static float azimuth = 0.0f;  // Ángulo de azimut (en radianes)
                static float elevation = 0.0f;  // Ángulo de elevación (en radianes)

                // Control para ángulo de azimut y elevación
                if (ImGui::SliderAngle("Azimuth", &azimuth, 0.0f, 360.0f))
                {
                    // Solo actualizar la posición del sol cuando cambian los ángulos
                    float cosElevation = cos(elevation);
                    Renderer::getInstance().dynamicSkybox->sunPosition.x = orbitRadius * cosElevation * cos(azimuth);
                    Renderer::getInstance().dynamicSkybox->sunPosition.y = orbitRadius * sin(elevation);
                    Renderer::getInstance().dynamicSkybox->sunPosition.z = orbitRadius * cosElevation * sin(azimuth);

                    Renderer::getInstance().dynamicSkybox->SyncSunLightWithSkybox();
                }

                if (ImGui::SliderAngle("Elevation", &elevation, -90.0f, 90.0f))
                {
                    // Solo actualizar la posición del sol cuando cambian los ángulos
                    float cosElevation = cos(elevation);
                    Renderer::getInstance().dynamicSkybox->sunPosition.x = orbitRadius * cosElevation * cos(azimuth);
                    Renderer::getInstance().dynamicSkybox->sunPosition.y = orbitRadius * sin(elevation);
                    Renderer::getInstance().dynamicSkybox->sunPosition.z = orbitRadius * cosElevation * sin(azimuth);

                    Renderer::getInstance().dynamicSkybox->SyncSunLightWithSkybox();
                }

                // Control del radio de la órbita
                if (ImGui::SliderFloat("Orbit Radius", &orbitRadius, 1.0f, 100.0f, "%.1f"))
                {
                    // Actualizar la posición del sol cuando cambia el radio
                    float cosElevation = cos(elevation);
                    Renderer::getInstance().dynamicSkybox->sunPosition.x = orbitRadius * cosElevation * cos(azimuth);
                    Renderer::getInstance().dynamicSkybox->sunPosition.y = orbitRadius * sin(elevation);
                    Renderer::getInstance().dynamicSkybox->sunPosition.z = orbitRadius * cosElevation * sin(azimuth);

                    Renderer::getInstance().dynamicSkybox->SyncSunLightWithSkybox();
                }

                ImGui::Dummy(ImVec2(0.0f, 5.0f));

                // Control del disco solar
                float sunDiskSizeValue = Renderer::getInstance().dynamicSkybox->m_sunDiskSize.x;
                if (ImGui::SliderFloat(ICON_FA_CIRCLE " Sun Disk Size", &sunDiskSizeValue, 0.0f, 1.0f, "%.4f"))
                {
                    Renderer::getInstance().dynamicSkybox->m_sunDiskSize = glm::vec3(sunDiskSizeValue);
                }
                ImGui::SliderFloat(ICON_FA_CIRCLE " Gradient Intensity", &Renderer::getInstance().dynamicSkybox->m_gradientIntensity, 0.0f, 10.0f, "%.4f");
                ImGui::SliderFloat(ICON_FA_CIRCLE " Aura Intensity", &Renderer::getInstance().dynamicSkybox->auraIntensity, 0.0f, 1.0f, "%.4f");
                ImGui::SliderFloat(ICON_FA_CIRCLE " Aura Size", &Renderer::getInstance().dynamicSkybox->auraSize, 0.0f, 1.0f, "%.4f");
                ImGui::SliderFloat(ICON_FA_CIRCLE " Edge Softness", &Renderer::getInstance().dynamicSkybox->edgeSoftness, 0.0001f, 0.1f, "%.4f");
                ImGui::Dummy(ImVec2(0.0f, 3.0f));
            }



            // Stars Section
            if (ImGui::CollapsingHeader(ICON_FA_STAR " Stars Settings"))
            {
                ImGui::SliderFloat("Star Density", &Renderer::getInstance().dynamicSkybox->starDensity, 0.0f, 0.01f, "%.5f");
                ImGui::SliderFloat("Star Size Min", &Renderer::getInstance().dynamicSkybox->starSizeMin, 0.0f, 1.0f, "%.4f");
                ImGui::SliderFloat("Star Size Max", &Renderer::getInstance().dynamicSkybox->starSizeMax, 0.0f, 2.0f, "%.4f");
                ImGui::SliderFloat("Star Brightness Min", &Renderer::getInstance().dynamicSkybox->starBrightnessMin, 0.0f, 1.0f, "%.4f");
                ImGui::SliderFloat("Star Brightness Max", &Renderer::getInstance().dynamicSkybox->starBrightnessMax, 0.0f, 1.0f, "%.4f");
                ImGui::DragFloat2("Star Coord Scale", glm::value_ptr(Renderer::getInstance().dynamicSkybox->starCoordScale), 0.1f, 0.0f, 200.0f, "%.4f");
            }


            // Light Section
            if (ImGui::CollapsingHeader(ICON_FA_LIGHTBULB " " ICON_FA_SUN " Sun Light")) {
                //ImGui::Text(ICON_FA_LIGHTBULB " " ICON_FA_SUN " Sun Light");
                ImGui::Separator();
                ImGui::Text(ICON_FA_RADIATION " Intensity");
                ImGui::DragFloat("##Intensity", &Renderer::getInstance().dynamicSkybox->sunLight->intensity, 0.1f, 0.0f, 1000.0f, "%.2f");
                ImGui::Separator();

                ImGui::Text(ICON_FA_ARROWS_ALT_H " Scene Radius Offset");
                if (ImGui::SliderFloat("##sceneRadiusOffset", &Renderer::getInstance().dynamicSkybox->sunLight->sceneRadiusOffset, 0.0f, 100.0f, "%.1f"))
                {
                    Renderer::getInstance().dynamicSkybox->sunLight->UpdateSceneRadius();
                }
                ImGui::Text(ICON_FA_ARROWS_ALT_V " Current Scene Radius");
                ImGui::SliderFloat("##currentSceneRadius", &Renderer::getInstance().dynamicSkybox->sunLight->currentSceneRadius, 0.0f, 100.0f, "%.1f");

                ImGui::Dummy(ImVec2(0.0f, 5.0f));
                ImGui::Text(ICON_FA_PALETTE " Ambient Color");
                ImGui::ColorEdit3("##Ambient", glm::value_ptr(Renderer::getInstance().dynamicSkybox->sunLight->ambient));

                ImGui::Text(ICON_FA_PALETTE " Diffuse Color");
                ImGui::ColorEdit3("##Diffuse", glm::value_ptr(Renderer::getInstance().dynamicSkybox->sunLight->diffuse));

                ImGui::Text(ICON_FA_PALETTE " Specular Color");
                ImGui::ColorEdit3("##Specular", glm::value_ptr(Renderer::getInstance().dynamicSkybox->sunLight->specular));

                ImGui::Dummy(ImVec2(0.0f, 5.0f));
                ImGui::Text(ICON_FA_SHIELD_ALT " Draw Shadows");
                ImGui::Checkbox("##DrawShadows", &Renderer::getInstance().dynamicSkybox->sunLight->drawShadows);


                if (Renderer::getInstance().dynamicSkybox->sunLight->drawShadows) {
                    ImGui::Text(ICON_FA_LIGHTBULB " Shadows");
                    ImGui::SliderFloat(ICON_FA_TINT " Shadow Intensity", &Renderer::getInstance().dynamicSkybox->sunLight->shadowIntensity, 0.0f, 1.0f);
                    ImGui::Checkbox(ICON_FA_BULLSEYE " Use Poison Disk", &Renderer::getInstance().dynamicSkybox->sunLight->usePoisonDisk);

                    ImGui::Dummy(ImVec2(0.0f, 20.0f));

                    ImGui::SliderFloat(ICON_FA_ARROW_LEFT " Left", &Renderer::getInstance().dynamicSkybox->sunLight->orthoLeft, -100.0f, 100.0f);
                    ImGui::SliderFloat(ICON_FA_ARROW_RIGHT " Right", &Renderer::getInstance().dynamicSkybox->sunLight->orthoRight, -100.0f, 100.0f);
                    ImGui::SliderFloat(ICON_FA_ARROW_UP " Top", &Renderer::getInstance().dynamicSkybox->sunLight->orthoTop, -100.0f, 100.0f);
                    ImGui::SliderFloat(ICON_FA_ARROW_DOWN " Bottom", &Renderer::getInstance().dynamicSkybox->sunLight->orthoBottom, -100.0f, 100.0f);

                    ImGui::SliderFloat(ICON_FA_ARROWS_ALT_V " Near", &Renderer::getInstance().dynamicSkybox->sunLight->orthoNear, 0.0f, 100.0f);
                    ImGui::SliderFloat(ICON_FA_COMPRESS_ARROWS_ALT " Near Offset", &Renderer::getInstance().dynamicSkybox->sunLight->orthoNearOffset, -100.0f, 100.0f);
                    ImGui::SliderFloat(ICON_FA_ARROWS_ALT_V " Far", &Renderer::getInstance().dynamicSkybox->sunLight->orthoFar, 0.0f, 500.0f);
                    ImGui::SliderFloat(ICON_FA_COMPRESS_ARROWS_ALT " Far Offset", &Renderer::getInstance().dynamicSkybox->sunLight->orthoFarOffset, -100.0f, 100.0f);

                    ImGui::Dummy(ImVec2(0.0f, 20.0f));


                    ImGui::Dummy(ImVec2(0.0f, 20.0f));
                    if (ImGui::CollapsingHeader("Shadow Bias")) {
                        ImGui::InputFloat("00", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[0][0], 0.001f);
                        ImGui::InputFloat("01", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[0][1], 0.001f);
                        ImGui::InputFloat("02", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[0][2], 0.001f);
                        ImGui::InputFloat("03", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[0][3], 0.001f);

                        ImGui::InputFloat("10", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[1][0], 0.001f);
                        ImGui::InputFloat("11", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[1][1], 0.001f);
                        ImGui::InputFloat("12", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[1][2], 0.001f);
                        ImGui::InputFloat("13", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[1][3], 0.001f);

                        ImGui::InputFloat("20", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[2][0], 0.001f);
                        ImGui::InputFloat("21", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[2][1], 0.001f);
                        ImGui::InputFloat("22", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[2][2], 0.001f);
                        ImGui::InputFloat("23", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[2][3], 0.001f);

                        ImGui::InputFloat("30", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[3][0], 0.001f);
                        ImGui::InputFloat("31", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[3][1], 0.001f);
                        ImGui::InputFloat("32", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[3][2], 0.001f);
                        ImGui::InputFloat("33", &Renderer::getInstance().dynamicSkybox->sunLight->shadowBias[3][3], 0.001f);

                        if (ImGui::Button("Reset")) {
                            Renderer::getInstance().dynamicSkybox->sunLight->shadowBias = glm::mat4(
                                0.5, 0.0, 0.0, 0.0,
                                0.0, 0.5, 0.0, 0.0,
                                0.0, 0.0, 0.5, 0.0,
                                0.5, 0.5, 0.5, 1.0
                            );
                        }
                    }
                }
            }

            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 5.0f));


            //--
            ImGui::End();
        }


        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}




