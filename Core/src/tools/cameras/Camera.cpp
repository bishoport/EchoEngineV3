#include "Camera.h"
#include "../../managers/ShaderManager.h"

namespace libCore
{
    Camera::Camera(int width, int height, glm::vec3 position)
    {
        this->width = width;
        this->height = height;
        this->Position = position;
        this->OrientationQuat = glm::quat(Orientation); // Inicializar cuaternión desde los ángulos de Euler

        setupFrustumBuffers();
    }

    void Camera::updateMatrix()
    {
        // Limitar el pitch para evitar el Gimbal Lock
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        // Actualizar la orientación usando los ángulos de Euler en yaw y pitch
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        Orientation = glm::normalize(front);

        // Actualizar el cuaternión
        OrientationQuat = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));

        this->view = glm::lookAt(Position, Position + Orientation, Up);

        // Seleccionar la matriz de proyección en función del modo
        if (isOrthographic)
        {
            float orthoSize = 10.0f; // Tamaño de la cámara ortográfica (ajustable)
            this->projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
        }
        else
        {
            this->projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);
        }

        this->cameraMatrix = projection * view;

        // Update Frustum
        UpdateFrustum();
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
        // Convertir la orientación de Euler a un cuaternión
        this->OrientationQuat = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f));
    }

    void Camera::UpdateOrientationFromQuaternion()
    {
        // Convertir el cuaternión a ángulos de Euler
        glm::vec3 euler = glm::eulerAngles(OrientationQuat);
        this->pitch = glm::degrees(euler.x);
        this->yaw = glm::degrees(euler.y);
    }

    void Camera::LookAt(const glm::vec3& targetPosition)
    {
        // Calcula la dirección hacia el objetivo
        glm::vec3 direction = glm::normalize(targetPosition - Position);

        // Calcula el nuevo pitch y yaw
        pitch = glm::degrees(asin(direction.y)); // Pitch: inclinación hacia arriba o abajo
        yaw = glm::degrees(atan2(direction.z, direction.x)); // Yaw: rotación alrededor del eje Y

        // Limitar el pitch para evitar el Gimbal Lock
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        // Ajustar la orientación usando los nuevos valores de pitch y yaw
        updateMatrix();
    }

    void Camera::UpdateFrustum()
    {
        glm::mat4 viewProjectionMatrix = this->projection * this->view;
        ExtractPlanes(viewProjectionMatrix);
    }

    void Camera::ExtractPlanes(const glm::mat4& viewProjectionMatrix)
    {
        // Left
        planes[Left].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0];
        planes[Left].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0];
        planes[Left].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0];
        planes[Left].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0];

        // Right
        planes[Right].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0];
        planes[Right].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0];
        planes[Right].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0];
        planes[Right].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0];

        // Bottom
        planes[Bottom].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1];
        planes[Bottom].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1];
        planes[Bottom].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1];
        planes[Bottom].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1];

        // Top
        planes[Top].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1];
        planes[Top].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1];
        planes[Top].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1];
        planes[Top].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1];

        // Near
        planes[Near].normal.x = viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2];
        planes[Near].normal.y = viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2];
        planes[Near].normal.z = viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2];
        planes[Near].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2];

        // Far
        planes[Far].normal.x = viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2];
        planes[Far].normal.y = viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2];
        planes[Far].normal.z = viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2];
        planes[Far].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2];

        // Normalize planes
        for (auto& plane : planes)
        {
            float length = glm::length(plane.normal);
            plane.normal /= length;
            plane.distance /= length;
        }
    }

    bool Camera::IsBoxInFrustum(const glm::vec3& min, const glm::vec3& max) const
    {
        for (const auto& plane : planes)
        {
            glm::vec3 p = min;

            if (plane.normal.x >= 0)
                p.x = max.x;
            if (plane.normal.y >= 0)
                p.y = max.y;
            if (plane.normal.z >= 0)
                p.z = max.z;

            if (plane.GetDistanceToPoint(p) < 0)
                return false;
        }
        return true;
    }

    void Camera::setupFrustumBuffers()
    {
        glGenVertexArrays(1, &frustumVAO);
        glGenBuffers(1, &frustumVBO);

        glBindVertexArray(frustumVAO);
        glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
    }

    void Camera::renderFrustumLines(const std::vector<glm::vec3>& vertices, const glm::mat4& modelMatrix)
    {
        glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        libCore::ShaderManager::Get("frustum")->use();
        libCore::ShaderManager::Get("frustum")->setMat4("view", view);
        libCore::ShaderManager::Get("frustum")->setMat4("projection", projection);
        libCore::ShaderManager::Get("frustum")->setMat4("model", modelMatrix);

        glLineWidth(2.0f); // Grosor de las líneas

        glBindVertexArray(frustumVAO);
        glDrawArrays(GL_LINES, 0, 24); // 24 vértices para dibujar las líneas del frustum
        glBindVertexArray(0);

        glLineWidth(1.0f); // Volver al grosor predeterminado si es necesario
    }

    void Camera::RenderFrustum()
    {
        std::vector<glm::vec3> vertices;

        // Esquinas del frustum en espacio NDC
        glm::vec4 corners[] =
        {
            { -1, -1, -1, 1 }, // near-bottom-left
            {  1, -1, -1, 1 }, // near-bottom-right
            {  1,  1, -1, 1 }, // near-top-right
            { -1,  1, -1, 1 }, // near-top-left
            { -1, -1,  1, 1 }, // far-bottom-left
            {  1, -1,  1, 1 }, // far-bottom-right
            {  1,  1,  1, 1 }, // far-top-right
            { -1,  1,  1, 1 }, // far-top-left
        };

        // Transformar las esquinas al espacio mundial usando la matriz de vista y proyección de la cámara
        glm::mat4 inverseVP = glm::inverse(projection * view);
        for (auto& corner : corners)
        {
            glm::vec4 worldPos = inverseVP * corner;
            vertices.push_back(glm::vec3(worldPos) / worldPos.w);
        }

        // Crear la matriz de modelo usando la posición y la orientación de la cámara
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, Position); // Aplicar la posición de la cámara
        modelMatrix *= glm::toMat4(OrientationQuat); // Aplicar la rotación de la cámara usando el cuaternión

        // Definir las líneas del frustum conectando las esquinas
        renderFrustumLines(vertices, modelMatrix);
    }
}

