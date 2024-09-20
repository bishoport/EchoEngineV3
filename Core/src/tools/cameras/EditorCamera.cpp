#include "EditorCamera.h"
#include "../../managers/WindowManager.h"
#include "../../managers/ViewportManager.hpp"
#include "../../gui/GuiLayer.h"

namespace libCore
{
    EditorCamera::EditorCamera(int width, int height, glm::vec3 position)
        : Camera(width, height, position)
    {
    }

    
    void EditorCamera::Inputs(libCore::Timestep deltaTime)
    {
        // Movimiento de la cámara
        float velocity = speed * deltaTime.GetMilliseconds();

        // Calcular los ejes locales de la cámara
        glm::vec3 forward = glm::normalize(Orientation);  // El eje forward es la orientación actual de la cámara
        glm::vec3 right = glm::normalize(glm::cross(forward, Up));  // El eje right es perpendicular a forward y Up
        glm::vec3 up = glm::normalize(glm::cross(right, forward));  // El eje Up local de la cámara

        // Control de rotación con el mouse
        if (InputManager::Instance().IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        {
            // Movimiento en la dirección de los ejes locales
            if (InputManager::Instance().IsKeyPressed(GLFW_KEY_W))
                Position += velocity * forward;   // Mover hacia adelante en el eje forward
            if (InputManager::Instance().IsKeyPressed(GLFW_KEY_S))
                Position -= velocity * forward;   // Mover hacia atrás en el eje forward
            if (InputManager::Instance().IsKeyPressed(GLFW_KEY_A))
                Position -= velocity * right;     // Mover hacia la izquierda en el eje right
            if (InputManager::Instance().IsKeyPressed(GLFW_KEY_D))
                Position += velocity * right;     // Mover hacia la derecha en el eje right
            if (InputManager::Instance().IsKeyPressed(GLFW_KEY_R))
                Position += velocity * up;        // Mover hacia arriba en el eje up
            if (InputManager::Instance().IsKeyPressed(GLFW_KEY_F))
                Position -= velocity * up;        // Mover hacia abajo en el eje up

        
            glfwSetInputMode(WindowManager::GetInstance().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            if (firstClick)
            {
                glfwSetCursorPos(WindowManager::GetInstance().GetWindow(), (width / 2), (height / 2));
                firstClick = false;
            }

            double mouseX, mouseY;
            glfwGetCursorPos(WindowManager::GetInstance().GetWindow(), &mouseX, &mouseY);

            float rotX = sensitivity * static_cast<float>(mouseY - (height / 2)) / height;
            float rotY = sensitivity * static_cast<float>(mouseX - (width / 2)) / width;

            // Ajustar pitch y yaw basados en la entrada del mouse
            pitch -= rotX;  // Invertir rotX para que el movimiento del mouse sea intuitivo
            yaw += rotY;

            // Limitar el pitch para evitar que la cámara rote demasiado hacia arriba o abajo
            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            // Actualizar la orientación de la cámara
            updateMatrix();

            // Resetear la posición del cursor
            glfwSetCursorPos(WindowManager::GetInstance().GetWindow(), (width / 2), (height / 2));
        }
        else
        {
            glfwSetInputMode(WindowManager::GetInstance().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstClick = true;
        }
    }















    ////------------------NO BORRAR-------------------------
    ////---ESTA ES LA VERSION BUENA PARA FULLSCREEN
    //void EditorCamera::Inputs(libCore::Timestep deltaTime)
    //{
    //    // Movimiento de la cámara
    //    float velocity = speed * deltaTime.GetMilliseconds();

    //    // Calcular los ejes locales de la cámara
    //    glm::vec3 forward = glm::normalize(Orientation);
    //    glm::vec3 right = glm::normalize(glm::cross(forward, Up));
    //    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    //    // Movimiento en la dirección de los ejes locales
    //    if (InputManager::Instance().IsKeyPressed(GLFW_KEY_W))
    //        Position += velocity * forward;
    //    if (InputManager::Instance().IsKeyPressed(GLFW_KEY_S))
    //        Position -= velocity * forward;
    //    if (InputManager::Instance().IsKeyPressed(GLFW_KEY_A))
    //        Position -= velocity * right;
    //    if (InputManager::Instance().IsKeyPressed(GLFW_KEY_D))
    //        Position += velocity * right;
    //    if (InputManager::Instance().IsKeyPressed(GLFW_KEY_R))
    //        Position += velocity * up;
    //    if (InputManager::Instance().IsKeyPressed(GLFW_KEY_F))
    //        Position -= velocity * up;

    //    // Control de rotación con el mouse
    //    if (InputManager::Instance().IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    //    {
    //        // Obtener las coordenadas del mouse dentro del viewport
    //        double mouseX, mouseY;
    //        glfwGetCursorPos(WindowManager::GetInstance().GetWindow(), &mouseX, &mouseY);

    //        // Ajustar la sensibilidad y la dirección de la rotación
    //        float rotX = sensitivity * static_cast<float>(mouseY - (height / 2)) / height;
    //        float rotY = sensitivity * static_cast<float>(mouseX - (width / 2)) / width;

    //        // Ajustar el pitch y yaw de la cámara
    //        pitch -= rotX;
    //        yaw += rotY;

    //        // Limitar el pitch para evitar el gimbal lock
    //        pitch = glm::clamp(pitch, -89.0f, 89.0f);

    //        // Actualizar la matriz de la cámara con la nueva orientación
    //        updateMatrix();

    //        // Reiniciar la posición del cursor al centro del viewport
    //        glfwSetCursorPos(WindowManager::GetInstance().GetWindow(), width / 2, height / 2);
    //    }
    //    else
    //    {
    //        firstClick = true;
    //    }
    //}

}
