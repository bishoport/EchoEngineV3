#include "GuiStyle.h"

namespace libCore
{
    void GuiStyle::ApplyCustomStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Configuración de colores
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.70f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.44f, 0.44f, 0.90f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.60f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.90f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.35f, 0.35f, 0.90f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 0.90f);

        // Modificación del estilo general
        style.FrameRounding = 5.0f;           // Redondeo de los bordes de los elementos
        style.GrabRounding = 5.0f;            // Redondeo de los controles deslizantes
        style.WindowRounding = 5.0f;          // Redondeo de las ventanas
        style.PopupRounding = 5.0f;           // Redondeo de las ventanas emergentes
        style.ScrollbarRounding = 5.0f;       // Redondeo de las barras de scroll
        style.ItemSpacing = ImVec2(10, 5);    // Espaciado entre los elementos
        style.FramePadding = ImVec2(5, 2);    // Relleno de los frames de botones y campos de entrada
    }
}

