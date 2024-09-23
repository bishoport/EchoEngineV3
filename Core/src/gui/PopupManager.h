#pragma once
#include <string>
#include <functional>
#include <vector>
#include "imgui.h"

class PopupManager
{
public:
    // Estructura de un Popup
    /*struct Popup {
        std::string title;
        bool isOpen;
        std::function<void()> content;

        Popup(const std::string& title, const std::function<void()>& content)
            : title(title), isOpen(false), content(content) {}
    };*/

    //Popups
    struct Popup {
        std::string title;
        bool isOpen;
        std::function<void()> content;

        Popup(const std::string& title, const std::function<void()>& content)
            : title(title), isOpen(false), content(content) {}
    };
    
    //-----------------------------------------------

    // Singleton - Obtener la instancia del PopupManager
    static PopupManager& GetInstance() {
        static PopupManager instance;
        return instance;
    }

    void RegisterPopup(const std::string& title, const std::function<void()>& content)
    {
        popups.emplace_back(title, content);
    }
    void ShowPopups()
    {
        for (auto& popup : popups) {
            if (popup.isOpen) {
                ImGui::OpenPopup(popup.title.c_str());
            }

            if (ImGui::BeginPopupModal(popup.title.c_str(), &popup.isOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
                popup.content();
                ImGui::EndPopup();
            }
        }
    }
    void OpenPopup(const std::string& title)
    {
        for (auto& popup : popups) {
            if (popup.title == title) {
                popup.isOpen = true;
                break;
            }
        }
    }
    void ClosePopup(const std::string& title)
    {
        for (auto& popup : popups) {
            if (popup.title == title) {
                popup.isOpen = false;
                break;
            }
        }
    }
    //-------------------------------------------------------------------------------

private:
    PopupManager() = default;

    // No permitir copia ni asignación
    PopupManager(const PopupManager&) = delete;
    void operator=(const PopupManager&) = delete;

    std::vector<Popup> popups;
};
