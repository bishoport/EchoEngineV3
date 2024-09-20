#pragma once

//#include "../core/Core.h"


namespace libCore
{
    enum class LogLevel {
        L_INFO,
        L_WARNING,
        L_ERROR,
        L_SUCCESS
    };

    struct LogEntry {
        LogLevel level;
        std::string message;
    };

    class ConsoleLog
    {
    public:
        static ConsoleLog& GetInstance() {
            static ConsoleLog instance;
            return instance;
        }

        void AddLog(LogLevel level, const std::string& message) {
            std::lock_guard<std::mutex> lock(mutex_);
            logs_.emplace_back(LogEntry{ level, message });
        }

        void ClearLogs() {
            std::lock_guard<std::mutex> lock(mutex_);
            logs_.clear();
        }

        const std::vector<LogEntry>& GetLogs() const {
            return logs_;
        }

        // Nueva función para manejar mensajes de OpenGL
        static void GLAPIENTRY MessageCallback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam)
        {
            auto& log = ConsoleLog::GetInstance();

            std::string logMessage = "GL CALLBACK: " + std::string(message);

            if (severity == GL_DEBUG_SEVERITY_HIGH) {
                log.AddLog(LogLevel::L_ERROR, logMessage);
            }
            else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
                //log.AddLog(LogLevel::L_WARNING, logMessage);
            }
            else if (severity == GL_DEBUG_SEVERITY_LOW) {
                log.AddLog(LogLevel::L_INFO, logMessage);
            }
            else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
                log.AddLog(LogLevel::L_INFO, "GL NOTIFICATION: " + std::string(message));
            }

            // Si quieres también mostrarlo por la consola, puedes usar esto:
            /*if (severity == GL_DEBUG_SEVERITY_HIGH) {
                std::cerr << "GL CALLBACK: HIGH SEVERITY ERROR - " << message << std::endl;
            }
            else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
                std::cerr << "GL CALLBACK: WARNING SEVERITY ERROR - " << logMessage << std::endl;
            }
            else if (severity == GL_DEBUG_SEVERITY_LOW) {
                std::cerr << "GL CALLBACK: LOW SEVERITY ERROR - " << logMessage << std::endl;
            }
            else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
                std::cerr << "GL CALLBACK: GL NOTIFICATION - " << std::string(message) << std::endl;
            }*/
        }

    private:
        ConsoleLog() = default;
        ~ConsoleLog() = default;

        ConsoleLog(const ConsoleLog&) = delete;
        ConsoleLog& operator=(const ConsoleLog&) = delete;

        std::vector<LogEntry> logs_;
        mutable std::mutex mutex_;
    };
}
