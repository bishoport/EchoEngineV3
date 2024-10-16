#pragma once
#include "PanelBase.h"
#include "../../core/renderer/Renderer.h"

namespace libCore
{
    class GlobalIluminationPanel : public PanelBase
    {
    public:
        GlobalIluminationPanel() : PanelBase("Global Ilumination") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            // Global Light Section
            ImGui::Text(ICON_FA_SUN " Global Light");
            ImGui::SliderFloat(ICON_FA_LIGHTBULB " Ambient Light", &Renderer::getInstance().ambientLight, 0.0f, 10.0f, "%.2f");
            ImGui::SliderFloat(ICON_FA_SUN " HDR Exposure", &Renderer::getInstance().hdrExposure, 0.1f, 10.0f, "%.2f");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            // IBL Section
            ImGui::Text(ICON_FA_GLOBE " IBL");
            ImGui::Checkbox(ICON_FA_CHECK_CIRCLE " Enable IBL", &Renderer::getInstance().iblEnabled);
            ImGui::SliderFloat(ICON_FA_SUN " Intensity", &Renderer::getInstance().iblIntensity, 0.0f, 10.0f, "%.2f");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            // SSAO Section
            ImGui::Text(ICON_FA_ADJUST " SSAO");
            ImGui::Checkbox(ICON_FA_CHECK_CIRCLE " Enable SSAO", &Renderer::getInstance().ssaoEnabled);
            ImGui::SliderFloat(ICON_FA_CIRCLE_NOTCH " SSAO Radius", &Renderer::getInstance().ssaoRadius, 0.1f, 10.0f, "%.2f");
            ImGui::SliderFloat(ICON_FA_CIRCLE_NOTCH " SSAO Bias", &Renderer::getInstance().ssaoBias, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat(ICON_FA_CIRCLE_NOTCH " SSAO Intensity", &Renderer::getInstance().ssaoIntensity, 0.0f, 5.0f, "%.2f");
            ImGui::SliderFloat(ICON_FA_CIRCLE_NOTCH " SSAO Power", &Renderer::getInstance().ssaoPower, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat(ICON_FA_LIGHTBULB " Base Reflectivity", &Renderer::getInstance().F0Factor, 0.1f, 5.0f, "%.2f");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            // Grid Section
            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            ImGui::Separator();
            ImGui::Text(ICON_FA_BORDER_ALL " Grid Settings");
            ImGui::SliderInt("Grid Size", &Renderer::getInstance().gridSize, 1, 100);
            ImGui::ColorEdit3("Grid Color", glm::value_ptr(Renderer::getInstance().gridColor));
            ImGui::SliderFloat("Grid Transparency", &Renderer::getInstance().gridTransparency, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Grid Cell Size", &Renderer::getInstance().gridCellSize, 0.1f, 10.0f, "%.2f");
            ImGui::SliderFloat("Grid Line Thickness", &Renderer::getInstance().gridCellLineSize, 0.1f, 5.0f, "%.2f");

            // Axis Section
            ImGui::Text(ICON_FA_COMPASS " Axis Settings");
            ImGui::ColorEdit3(ICON_FA_ARROW_RIGHT " X Axis Color", glm::value_ptr(Renderer::getInstance().axisXColor));
            ImGui::ColorEdit3(ICON_FA_ARROW_UP " Y Axis Color", glm::value_ptr(Renderer::getInstance().axisYColor));
            ImGui::ColorEdit3(ICON_FA_ARROW_LEFT " Z Axis Color", glm::value_ptr(Renderer::getInstance().axisZColor));
            ImGui::SliderFloat("Axis Size", &Renderer::getInstance().axisSize, 0.01f, 1.0f);
            ImGui::SliderFloat3("Axis Offset", glm::value_ptr(Renderer::getInstance().axisOffset), -5.0f, 5.0f);

            //--
            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}
