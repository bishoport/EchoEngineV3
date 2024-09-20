#pragma once

#include "../../core/Core.h"

#include "../../core/Timestep.h"
#include "../../managers/InputManager.h"


namespace libCore
{
    class Camera
    {
    public:
        glm::vec3 Position;
        glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f); // Ángulos de Euler
        glm::quat OrientationQuat = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)); // Cuaternión
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 cameraMatrix = glm::mat4(1.0f);

        int width;
        int height;

        float speed = 0.06f;
        float sensitivity = 100.0f;

        float yaw = -90.0f;   // Inicialmente apuntando hacia el frente
        float pitch = 0.0f;   // Inicialmente sin inclinación
        float FOVdeg = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;

        bool isOrthographic = false; // Controla si la cámara está en modo ortográfico

        Camera(int width, int height, glm::vec3 position);

        void updateMatrix();
        virtual void Inputs(libCore::Timestep deltaTime) = 0; // Método virtual puro para entradas específicas

        void UpdateOrientationFromEuler();
        void UpdateOrientationFromQuaternion();

        // Nueva función para que la cámara mire a un punto específico
        void LookAt(const glm::vec3& targetPosition);

        // Setters para actualizar propiedades desde fuera
        void SetPosition(const glm::vec3& position);
        void SetYaw(float yaw);
        void SetPitch(float pitch);
        void SetOrientationQuat(const glm::quat& orientationQuat);
        void SetUpVector(const glm::vec3& up);
        void SetFOV(float fov);
        void SetNearPlane(float nearPlane);
        void SetFarPlane(float farPlane);
        void SetSpeed(float speed);
        void SetSensitivity(float sensitivity);

        // Frustum related methods
        void UpdateFrustum();
        bool IsBoxInFrustum(const glm::vec3& min, const glm::vec3& max) const;
        void RenderFrustum();

    private:
        struct Plane
        {
            glm::vec3 normal;
            float distance;

            Plane() = default;
            Plane(const glm::vec3& normal, float distance)
                : normal(normal), distance(distance) {}

            float GetDistanceToPoint(const glm::vec3& point) const
            {
                return glm::dot(normal, point) + distance;
            }
        };

        enum Planes
        {
            Near = 0,
            Far,
            Left,
            Right,
            Top,
            Bottom,
            Count
        };

        std::array<Plane, Planes::Count> planes;

        void ExtractPlanes(const glm::mat4& viewProjectionMatrix);

        // OpenGL Buffers
        GLuint frustumVAO, frustumVBO;
        void setupFrustumBuffers();
        void renderFrustumLines(const std::vector<glm::vec3>& vertices, const glm::mat4& modelMatrix);
    };
}
