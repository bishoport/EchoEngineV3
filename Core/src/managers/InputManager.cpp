#include "InputManager.h"

namespace libCore
{
    InputManager& InputManager::Instance() {
        static InputManager instance;
        return instance;
    }

    void InputManager::Update() {
        // Llama a los callbacks suscritos si la tecla/botón correspondiente está presionado.
        for (auto& [key, callbacks] : m_keySubscribers) {
            if (m_keyStates[key]) {
                for (auto& callback : callbacks) {
                    callback();
                }
            }
        }
        for (auto& [button, callbacks] : m_mouseSubscribers) {
            if (m_mouseStates.count(button)) {
                auto [x, y] = m_mouseStates[button];
                for (auto& callback : callbacks) {
                    callback(x, y);
                }
            }
        }
    }

    bool InputManager::IsKeyPressed(int key) const {
        auto it = m_keyStates.find(key);
        if (it != m_keyStates.end())
            return it->second;
        return false;
    }

    bool InputManager::IsKeyJustPressed(int key) const {
        auto it = m_keyStates.find(key);
        auto lastIt = m_lastKeyStates.find(key);
        if (it != m_keyStates.end() && lastIt != m_lastKeyStates.end())
            return it->second && !lastIt->second;
        return false;
    }

    bool InputManager::IsMouseButtonDown(int button) const {
        return m_mouseStates.count(button) > 0;
    }

    bool InputManager::IsMouseButtonJustPressed(int button) const {
        bool isPressedNow = m_mouseStates.count(button) > 0;
        bool wasPressedBefore = m_lastMouseStates.count(button) > 0;
        return isPressedNow && !wasPressedBefore;
    }

    bool InputManager::IsMouseButtonJustReleased(int button) const
    {
        bool isPressedNow = m_mouseStates.count(button) > 0;
        bool wasPressedBefore = m_lastMouseStates.count(button) > 0;
        return !isPressedNow && wasPressedBefore;
    }

    void InputManager::EndFrame() {
        m_lastKeyStates = m_keyStates;
        m_lastMouseStates = m_mouseStates;
    }



    void InputManager::SetKeyPressed(int key, bool isPressed) {
        m_keyStates[key] = isPressed;
    }
    //
    void InputManager::SetMousePressed(int button, double x, double y, bool isPressed) {
        if (isPressed) {
            m_mouseStates[button] = { x, y };
        }
        else {
            m_mouseStates.erase(button);
        }
    }



    void InputManager::SetMousePosition(double x, double y) {
        m_lastMousePosition = m_mousePosition;
        m_mousePosition = { x, y };
    }

    void InputManager::SetViewportSize(int width, int height) {
        m_viewportSize = { width, height };
    }

    std::pair<double, double> InputManager::GetMousePosition() const {
        return m_mousePosition;
    }

    std::pair<double, double> InputManager::GetMouseDelta() const {
        return { m_mousePosition.first - m_lastMousePosition.first,
                 m_mousePosition.second - m_lastMousePosition.second };
    }
}
