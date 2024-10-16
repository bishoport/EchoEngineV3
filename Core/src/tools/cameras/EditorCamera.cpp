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
        // Movimiento de la c�mara
        float velocity = speed * deltaTime.GetMilliseconds();

        // Control de rotaci�n con el mouse
        if (InputManager::Instance().IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        {
            // Ocultar el cursor y deshabilitar el movimiento del mismo
            glfwSetInputMode(WindowManager::GetInstance().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            if (firstClick)
            {
                // Establecer la posici�n del cursor en el centro del viewport la primera vez
                glfwSetCursorPos(WindowManager::GetInstance().GetWindow(), (width / 2), (height / 2));
                firstClick = false;
            }

            double mouseX, mouseY;
            glfwGetCursorPos(WindowManager::GetInstance().GetWindow(), &mouseX, &mouseY);

            // Calcular las variaciones en el eje X e Y del mouse
            float rotX = sensitivity * static_cast<float>(mouseY - (height / 2)) / height;
            float rotY = sensitivity * static_cast<float>(mouseX - (width / 2)) / width;

            // Ajustar pitch y yaw basados en la entrada del mouse
            pitch -= rotX;  // Invertir rotX para que el movimiento del mouse sea intuitivo
            yaw += rotY;

            // Limitar el pitch para evitar que la c�mara rote demasiado hacia arriba o abajo
            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            // Actualizar la orientaci�n de la c�mara y los vectores relacionados
            updateMatrix();

            // Resetear la posici�n del cursor al centro del viewport
            glfwSetCursorPos(WindowManager::GetInstance().GetWindow(), (width / 2), (height / 2));
        }
        else
        {
            // Mostrar el cursor cuando no se est� presionando el bot�n derecho
            glfwSetInputMode(WindowManager::GetInstance().GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstClick = true;
        }

        // Movimiento de la c�mara en la direcci�n de los ejes locales
        if (InputManager::Instance().IsKeyPressed(GLFW_KEY_W))
            Position += velocity * Front;   // Mover hacia adelante
        if (InputManager::Instance().IsKeyPressed(GLFW_KEY_S))
            Position -= velocity * Front;   // Mover hacia atr�s
        if (InputManager::Instance().IsKeyPressed(GLFW_KEY_A))
            Position -= velocity * Right;   // Mover hacia la izquierda
        if (InputManager::Instance().IsKeyPressed(GLFW_KEY_D))
            Position += velocity * Right;   // Mover hacia la derecha
        if (InputManager::Instance().IsKeyPressed(GLFW_KEY_R))
            Position += velocity * Up;      // Mover hacia arriba
        if (InputManager::Instance().IsKeyPressed(GLFW_KEY_F))
            Position -= velocity * Up;      // Mover hacia abajo

        // Actualizar la matriz de vista despu�s de cualquier movimiento de posici�n
        updateMatrix();
    }
}
















    ////------------------NO BORRAR-------------------------
    ////---ESTA ES LA VERSION BUENA PARA FULLSCREEN
    //void EditorCamera::Inputs(libCore::Timestep deltaTime)
    //{
    //    // Movimiento de la c�mara
    //    float velocity = speed * deltaTime.GetMilliseconds();

    //    // Calcular los ejes locales de la c�mara
    //    glm::vec3 forward = glm::normalize(Orientation);
    //    glm::vec3 right = glm::normalize(glm::cross(forward, Up));
    //    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    //    // Movimiento en la direcci�n de los ejes locales
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

    //    // Control de rotaci�n con el mouse
    //    if (InputManager::Instance().IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    //    {
    //        // Obtener las coordenadas del mouse dentro del viewport
    //        double mouseX, mouseY;
    //        glfwGetCursorPos(WindowManager::GetInstance().GetWindow(), &mouseX, &mouseY);

    //        // Ajustar la sensibilidad y la direcci�n de la rotaci�n
    //        float rotX = sensitivity * static_cast<float>(mouseY - (height / 2)) / height;
    //        float rotY = sensitivity * static_cast<float>(mouseX - (width / 2)) / width;

    //        // Ajustar el pitch y yaw de la c�mara
    //        pitch -= rotX;
    //        yaw += rotY;

    //        // Limitar el pitch para evitar el gimbal lock
    //        pitch = glm::clamp(pitch, -89.0f, 89.0f);

    //        // Actualizar la matriz de la c�mara con la nueva orientaci�n
    //        updateMatrix();

    //        // Reiniciar la posici�n del cursor al centro del viewport
    //        glfwSetCursorPos(WindowManager::GetInstance().GetWindow(), width / 2, height / 2);
    //    }
    //    else
    //    {
    //        firstClick = true;
    //    }
    //}


