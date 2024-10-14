#pragma once
#include "PanelBase.h"
#include "../../managers/LuaManager.h"


namespace libCore
{
    class LuaPanel : public PanelBase
    {
    public:
        LuaPanel() : PanelBase("Scripts") {}

        void Init() override
        {

        }

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            // Obtener la lista de scripts cargados desde LuaManager
            auto loadedScripts = LuaManager::GetInstance().GetLoadedScripts();

            if (loadedScripts.empty()) {
                ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE " No Lua scripts loaded.");
            }
            else {
                ImGui::Text(ICON_FA_FILE_CODE " Loaded Lua Scripts:");

                // Crear una tabla con dos columnas: nombre y acciones
                if (ImGui::BeginTable("LuaScriptsTable", 3)) {
                    ImGui::TableSetupColumn("NAME");
                    ImGui::TableSetupColumn("PATH");
                    ImGui::TableSetupColumn("ACTIONS");
                    ImGui::TableHeadersRow();

                    // Iterar sobre los scripts cargados
                    for (const auto& scriptData : loadedScripts) {
                        ImGui::TableNextRow();

                        // Mostrar el nombre del script
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(ICON_FA_FILE " %s", scriptData.name.c_str());

                        // Mostrar el path del script
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(ICON_FA_FOLDER_OPEN " %s", scriptData.filePath.c_str());

                        // Crear el botón "Run Script" para ejecutar el script
                        ImGui::TableSetColumnIndex(2);
                        if (ImGui::Button((ICON_FA_PLAY " Run " + scriptData.name).c_str())) {
                            // Ejecutar el script al presionar el botón
                            try {
                                LuaManager::GetInstance().GetLuaState(scriptData.name).script_file(scriptData.filePath);
                                ImGui::Text(ICON_FA_CHECK_CIRCLE " Script executed successfully!");
                            }
                            catch (const sol::error& e) {
                                ImGui::TextColored(ImVec4(1, 0, 0, 1), ICON_FA_TIMES_CIRCLE " Error: %s", e.what());
                            }
                        }
                    }

                    ImGui::EndTable();
                }
            }

            //--
            ImGui::End();
        }


        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}