#include "Camera.h"
#include "../../managers/ShaderManager.h"

namespace libCore
{
    Camera::Camera(int width, int height, glm::vec3 position)
        : width(width), height(height), Position(position), WorldUp(Up)
    {
        OrientationQuat = glm::quat(Orientation);

        // Crear el frustum inicial utilizando los parámetros actuales de la cámara
        frustum = createFrustumFromCamera(static_cast<float>(width) / height, glm::radians(FOVdeg), nearPlane, farPlane);

        // Actualizar la matriz de la cámara para sincronizarla con los valores iniciales
        updateMatrix();
    }


    void Camera::updateMatrix()
    {
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        updateCameraVectors();

        view = glm::lookAt(Position, Position + Front, Up);

        if (isOrthographic)
        {
            float orthoSize = 10.0f;
            projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
        }
        else
        {
            projection = glm::perspective(glm::radians(FOVdeg), static_cast<float>(width) / height, nearPlane, farPlane);
        }

        cameraMatrix = projection * view;

        // Actualizar el frustum después de actualizar la cámara
        updateFrustum();
    }

    void Camera::updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }


    void Camera::SetPosition(const glm::vec3& position)
    {
        Position = position;
        updateMatrix();
    }

    void Camera::SetYaw(float newYaw)
    {
        yaw = newYaw;
        updateMatrix();
    }

    void Camera::SetPitch(float newPitch)
    {
        pitch = newPitch;
        updateMatrix();
    }

    void Camera::SetOrientationQuat(const glm::quat& orientationQuat)
    {
        OrientationQuat = orientationQuat;
        UpdateOrientationFromQuaternion();
        updateMatrix();
    }

    void Camera::SetUpVector(const glm::vec3& up)
    {
        Up = up;
        updateMatrix();
    }

    void Camera::SetFOV(float fov)
    {
        FOVdeg = fov;
        updateMatrix();
    }

    void Camera::SetNearPlane(float newNearPlane)
    {
        nearPlane = newNearPlane;
        updateMatrix();
    }

    void Camera::SetFarPlane(float newFarPlane)
    {
        farPlane = newFarPlane;
        updateMatrix();
    }

    void Camera::SetSpeed(float newSpeed)
    {
        speed = newSpeed;
    }

    void Camera::SetSensitivity(float newSensitivity)
    {
        sensitivity = newSensitivity;
    }

    void Camera::UpdateOrientationFromEuler()
    {
        OrientationQuat = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));
    }

    void Camera::UpdateOrientationFromQuaternion()
    {
        glm::vec3 euler = glm::eulerAngles(OrientationQuat);
        pitch = glm::degrees(euler.x);
        yaw = glm::degrees(euler.y);
    }

    void Camera::LookAt(const glm::vec3& targetPosition)
    {
        glm::vec3 direction = glm::normalize(targetPosition - Position);
        yaw = glm::degrees(atan2(direction.x, -direction.z));
        pitch = glm::degrees(asin(direction.y));
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        updateMatrix();
    }

    Frustum Camera::createFrustumFromCamera(float aspect, float fovY, float zNear, float zFar)
    {
        Frustum frustum;
        const float halfVSide = zFar * tanf(fovY * 0.5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * Front;

        frustum.nearFace = { Position + zNear * Front, Front };
        frustum.farFace = { Position + frontMultFar, -Front };
        frustum.rightFace = { Position, glm::cross(Up, frontMultFar + Right * halfHSide) };
        frustum.leftFace = { Position, glm::cross(frontMultFar - Right * halfHSide, Up) };
        frustum.topFace = { Position, glm::cross(Right, frontMultFar - Up * halfVSide) };
        frustum.bottomFace = { Position, glm::cross(frontMultFar + Up * halfVSide, Right) };

        return frustum;
    }

    bool Camera::IsBoxInFrustum(const glm::mat4& transform, const glm::vec3& min, const glm::vec3& max) const
    {
        // Obtener el centro del AABB en espacio local
        glm::vec3 localCenter = (min + max) * 0.5f;

        // Transformar el centro al espacio global
        glm::vec3 globalCenter = glm::vec3(transform * glm::vec4(localCenter, 1.0f));

        // Calcular las extensiones del AABB en espacio local
        glm::vec3 localExtents = (max - min) * 0.5f;

        // Obtener las direcciones escaladas en el espacio global usando la matriz de transformación
        glm::vec3 right = glm::vec3(transform[0]) * localExtents.x;
        glm::vec3 up = glm::vec3(transform[1]) * localExtents.y;
        glm::vec3 forward = glm::vec3(transform[2]) * localExtents.z;

        // Verificar cada plano del frustum
        for (const auto& plane : { frustum.nearFace, frustum.farFace, frustum.leftFace, frustum.rightFace, frustum.topFace, frustum.bottomFace })
        {
            // Calcular el radio proyectado del AABB en el plano actual
            float r = std::abs(glm::dot(plane.normal, right)) +
                std::abs(glm::dot(plane.normal, up)) +
                std::abs(glm::dot(plane.normal, forward));

            // Calcular la distancia del centro del AABB al plano
            float d = plane.getSignedDistanceToPlane(globalCenter);

            // Si la distancia es menor que el radio proyectado, el AABB está completamente fuera de este plano
            if (d < -r)
            {
                return false; // AABB está fuera del frustum
            }
        }

        // Si el AABB no está completamente fuera de ningún plano, está dentro (o intersectando)
        return true;
    }
    void Camera::updateFrustum()
    {
        const float aspect = static_cast<float>(width) / height;
        const float fovY = glm::radians(FOVdeg);
        const float zNear = nearPlane;
        const float zFar = farPlane;

        const float halfVSide = zFar * tanf(fovY * 0.5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * Front;

        frustum.nearFace = { Position + zNear * Front, Front };
        frustum.farFace = { Position + frontMultFar, -Front };
        frustum.rightFace = { Position, glm::cross(Up, frontMultFar + Right * halfHSide) };
        frustum.leftFace = { Position, glm::cross(frontMultFar - Right * halfHSide, Up) };
        frustum.topFace = { Position, glm::cross(Right, frontMultFar - Up * halfVSide) };
        frustum.bottomFace = { Position, glm::cross(frontMultFar + Up * halfVSide, Right) };
    }
}