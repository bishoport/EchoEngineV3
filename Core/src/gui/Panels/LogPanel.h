#pragma once
#include "PanelBase.h"

#include "../../core/Engine.h"
#include "../tools/ConsoleLog.h"

namespace libCore
{
    class LogPanel : public PanelBase
    {
    public:
        LogPanel() : PanelBase("Console") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            static bool showInfo = true;
            static bool showWarning = true;
            static bool showError = true;
            static bool showSuccess = true;  // Añadir checkbox para L_SUCCESS

            ImGui::Begin(m_title.c_str());
            //-- 

            // Filtros para los tipos de log
            if (ImGui::Button(ICON_FA_TRASH " Clear")) {
                ConsoleLog::GetInstance().ClearLogs();
            }

            ImGui::SameLine();
            ImGui::Checkbox(ICON_FA_INFO_CIRCLE " Info", &showInfo);
            ImGui::SameLine();
            ImGui::Checkbox(ICON_FA_EXCLAMATION_TRIANGLE " Warning", &showWarning);
            ImGui::SameLine();
            ImGui::Checkbox(ICON_FA_TIMES_CIRCLE " Error", &showError);
            ImGui::SameLine();
            ImGui::Checkbox(ICON_FA_CHECK_CIRCLE " Success", &showSuccess);  // Checkbox para L_SUCCESS

            ImGui::Separator();

            // Mostrar los logs
            ImGui::BeginChild("LogScrolling");
            const auto& logs = ConsoleLog::GetInstance().GetLogs();
            for (const auto& log : logs) {
                bool show = false;
                ImVec4 color;
                const char* icon = "";

                switch (log.level) {
                case LogLevel::L_INFO:
                    show = showInfo;
                    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // Blanco
                    icon = ICON_FA_INFO_CIRCLE;
                    break;
                case LogLevel::L_WARNING:
                    show = showWarning;
                    color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Amarillo
                    icon = ICON_FA_EXCLAMATION_TRIANGLE;
                    break;
                case LogLevel::L_ERROR:
                    show = showError;
                    color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Rojo
                    icon = ICON_FA_TIMES_CIRCLE;
                    break;
                case LogLevel::L_SUCCESS:
                    show = showSuccess;
                    color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Verde
                    icon = ICON_FA_CHECK_CIRCLE;
                    break;
                }

                if (show) {
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("%s %s", icon, log.message.c_str());
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();

            //-- 
            ImGui::End();
        }


        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}
