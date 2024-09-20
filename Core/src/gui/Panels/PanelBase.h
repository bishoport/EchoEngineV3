#pragma once
#include "../../core/Core.h"
#include <imgui/imgui.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

namespace libCore
{
    class PanelBase
    {
    public:
        // Constructor y destructor
        PanelBase(const std::string& title, bool isVisible = true) : m_title(title), m_isVisible(isVisible) {}
        virtual ~PanelBase() = default;

        // Funciones básicas que deben implementar las clases derivadas
        virtual void Init() = 0;
        virtual void Draw() = 0;
        virtual void Shutdown() = 0;

        // Mostrar/ocultar el panel
        void SetVisible(bool visible) { m_isVisible = visible; }
        bool IsVisible() const { return m_isVisible; }

        // Obtener el título del panel
        const std::string& GetTitle() const { return m_title; }

    protected:
        std::string m_title;   // Título del panel
        bool m_isVisible;      // Estado de visibilidad del panel
    };
}

