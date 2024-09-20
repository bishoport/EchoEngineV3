#pragma once
#include "PanelBase.h"

#include "../../managers/ViewportManager.hpp"
#include "../../managers/EventManager.h"

namespace libCore
{
    class EditorCameraPanel : public PanelBase
    {
    public:
        EditorCameraPanel() : PanelBase("Editor Camera") {}

        void Init() override{}

        void Draw() override {
            if (!m_isVisible) return;

            ImGui::Begin(m_title.c_str());
            //--

            // Obtén una referencia a la cámara del editor
            auto& editorCamera = ViewportManager::GetInstance().viewports[0]->camera;

            // Mostrar y permitir editar la posición de la cámara
            ImGui::Text(ICON_FA_MAP_MARKER_ALT " Position");
            glm::vec3 position = editorCamera->Position;
            if (ImGui::DragFloat3("##Position", glm::value_ptr(position), 0.1f))
            {
                editorCamera->SetPosition(position);
            }

            // Mostrar y permitir editar la orientación en ángulos de Euler (Yaw, Pitch)
            ImGui::Text(ICON_FA_COMPASS " Euler Angles");
            float yaw = editorCamera->yaw;
            float pitch = editorCamera->pitch;
            if (ImGui::DragFloat(ICON_FA_ARROWS_ALT " Yaw", &yaw, 0.1f, -180.0f, 180.0f))
            {
                editorCamera->SetYaw(yaw);
            }
            if (ImGui::DragFloat(ICON_FA_ARROWS_ALT " Pitch", &pitch, 0.1f, -89.0f, 89.0f))
            {
                editorCamera->SetPitch(pitch);
            }

            // Mostrar y permitir editar el cuaternión de orientación
            ImGui::Text(ICON_FA_SYNC_ALT " Orientation Quaternion");
            glm::quat orientationQuat = editorCamera->OrientationQuat;
            if (ImGui::DragFloat4("##OrientationQuat", glm::value_ptr(orientationQuat), 0.1f))
            {
                editorCamera->SetOrientationQuat(orientationQuat);
            }

            // Mostrar y permitir editar la dirección "Up" de la cámara
            ImGui::Text(ICON_FA_ARROW_UP " Up");
            glm::vec3 up = editorCamera->Up;
            if (ImGui::DragFloat3("##Up", glm::value_ptr(up), 0.1f))
            {
                editorCamera->SetUpVector(up);
            }

            // Mostrar y permitir editar el FOV de la cámara
            ImGui::Text(ICON_FA_EXPAND " Field of View");
            float fov = editorCamera->FOVdeg;
            if (ImGui::SliderFloat("##FOV", &fov, 1.0f, 120.0f))
            {
                editorCamera->SetFOV(fov);
            }

            // Mostrar y permitir editar los planos cercano y lejano
            ImGui::Text(ICON_FA_COMPRESS " Near Plane");
            float nearPlane = editorCamera->nearPlane;
            if (ImGui::DragFloat("##NearPlane", &nearPlane, 0.01f, 0.01f, 10.0f))
            {
                editorCamera->SetNearPlane(nearPlane);
            }

            ImGui::Text(ICON_FA_EXPAND_ARROWS_ALT " Far Plane");
            float farPlane = editorCamera->farPlane;
            if (ImGui::DragFloat("##FarPlane", &farPlane, 1.0f, 10.0f, 10000.0f))
            {
                editorCamera->SetFarPlane(farPlane);
            }

            // Mostrar y permitir editar la velocidad de la cámara
            ImGui::Text(ICON_FA_TACHOMETER_ALT " Speed");
            float speed = editorCamera->speed;
            if (ImGui::DragFloat("##Speed", &speed, 0.01f, 0.0f, 10.0f))
            {
                editorCamera->SetSpeed(speed);
            }

            // Mostrar y permitir editar la sensibilidad de la cámara
            ImGui::Text(ICON_FA_COG " Sensitivity");
            float sensitivity = editorCamera->sensitivity;
            if (ImGui::DragFloat("##Sensitivity", &sensitivity, 1.0f, 0.0f, 1000.0f))
            {
                editorCamera->SetSensitivity(sensitivity);
            }

            // Botón para cambiar entre ortográfico y perspectiva
            if (ImGui::Button(editorCamera->isOrthographic ? ICON_FA_VIDEO " Switch to Perspective" : ICON_FA_VIDEO_SLASH " Switch to Orthographic"))
            {
                editorCamera->isOrthographic = !editorCamera->isOrthographic;
                editorCamera->updateMatrix(); // Actualizar la matriz de la cámara inmediatamente
            }
            //--
            ImGui::End();
        }

        void Shutdown() override {
            // Liberación de recursos si es necesario
        }
    };
}
