#pragma once

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>

namespace libCore {
    class SceneHistoryManager {
    public:
        // Método para obtener la única instancia de SceneHistoryManager
        static SceneHistoryManager& GetInstance() {
            static SceneHistoryManager instance;
            return instance;
        }

        // Agregar una escena al historial y guardarlo
        void AddSceneToHistory(const std::string& scenePath) {
            if (std::find(sceneHistory.begin(), sceneHistory.end(), scenePath) == sceneHistory.end()) {
                sceneHistory.push_back(scenePath);
                SaveHistory();
            }
        }

        // Obtener el historial de escenas
        const std::vector<std::string>& GetSceneHistory() const {
            return sceneHistory;
        }

        // Cargar el historial de escenas desde un archivo
        void LoadHistory() {
            if (std::filesystem::exists("scene_history.yaml")) {
                YAML::Node data = YAML::LoadFile("scene_history.yaml");
                for (const auto& scene : data["Scenes"]) {
                    sceneHistory.push_back(scene.as<std::string>());
                }
            }
        }

        // Guardar el historial de escenas en un archivo
        void SaveHistory() {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Scenes" << YAML::Value << YAML::BeginSeq;
            for (const auto& scene : sceneHistory) {
                out << scene;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;

            std::ofstream fout("scene_history.yaml");
            fout << out.c_str();
        }

    private:
        // Constructor privado para el patrón Singleton
        SceneHistoryManager() {
            LoadHistory();
        }

        // Evitar copias y asignaciones del singleton
        SceneHistoryManager(const SceneHistoryManager&) = delete;
        SceneHistoryManager& operator=(const SceneHistoryManager&) = delete;

        // Historial de escenas
        std::vector<std::string> sceneHistory;
    };
}
