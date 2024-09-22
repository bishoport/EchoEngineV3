#pragma once

#include "../core/Core.h"


namespace libCore
{
    template <typename... Args>
    class Event {
    public:
        using CallbackType = std::function<void(Args...)>;

        void subscribe(const CallbackType& callback) {
            callbacks.push_back(callback);
        }

        void unsubscribe(const CallbackType& callback) {
            auto it = std::find(callbacks.begin(), callbacks.end(), callback);
            if (it != callbacks.end()) {
                callbacks.erase(it);
            }
        }

        void trigger(Args... args) const {
            for (const auto& callback : callbacks) {
                callback(args...);
            }
        }

    private:
        std::vector<CallbackType> callbacks;
    };




    class EventManager {
    public:

        //WINDOW
        static Event<int, int>& OnWindowResizeEvent() {
            static Event<int, int> event;
            return event;
        }
        static Event<int, int>& OnWindowMovedEvent() {
            static Event<int, int> event;
            return event;
        }
        //-----------------

        //PANEL EVENTS
        static Event<const std::string&, const glm::vec2&, const  glm::vec2&>& OnPanelResizedEvent() {
            static Event<const std::string&, const  glm::vec2&, const  glm::vec2&> event;
            return event;
        }
        static Event<const std::string&, bool>& OnPanelMouseEnterExitEvent() {
            static Event<const std::string&, bool> event; // bool indica si el mouse ha entrado (true) o salido (false)
            return event;
        }
        static Event<const std::string&, bool>& OnPanelFocusEvent() {
            static Event<const std::string&, bool> event; // bool indica si el panel tiene foco (true) o lo ha perdido (false)
            return event;
        }
        //-----------------


        //CARGA DE ASSETS
        static Event<const std::string&>& OnLoadAssetStart() {
            static Event<const std::string&> event;  // Archivo que se está cargando
            return event;
        }

        static Event<float>& OnLoadAssetProgress() {
            static Event<float> event;  // Progreso de la carga y la textura
            return event;
        }

        static Event<Ref<Texture>, bool>& OnLoadAssetComplete() {
            static Event<Ref<Texture>, bool> event;  // Archivo y si fue exitoso
            return event;
        }

        static Event<const std::string&, const std::string&>& OnLoadAssetFailed() {
            static Event<const std::string&, const std::string&> event;  // Archivo y el error
            return event;
        }
        //-----------------
    };
}
