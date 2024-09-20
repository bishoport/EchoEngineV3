#pragma once

#include <GLFW/glfw3.h>
#include <iostream>
#include "EventManager.h"
#include "InputManager.h"  // Incluye el InputManager

namespace libCore
{
    class WindowManager {
    public:
        static WindowManager& GetInstance() {
            static WindowManager instance;
            return instance;
        }

        bool Initialize(int width, int height, const char* title)
        {
            windowWidth = width;
            windowHeight = height;

            if (!glfwInit()) {
                return false;
            }


            glfwInit();

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

            window = glfwCreateWindow(windowWidth, windowHeight, title, nullptr, nullptr);
            if (window == nullptr) {
                std::cout << "Failed to create GLFW window" << std::endl;
                ConsoleLog::GetInstance().AddLog(LogLevel::L_ERROR, "Failed to create GLFW window ");
                glfwTerminate();
                return false;
            }
            glfwMakeContextCurrent(window);

            // Important: Set the swap interval to synchronize buffer swaps with the monitor's refresh rate
            //glfwSwapInterval(1); // Enable VSync

            SetupInputCallbacks();
            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
            glfwSetWindowPosCallback(window, window_pos_callback); // Callback para cambio de posición

            // Obtener la posición inicial de la ventana
            //glfwGetWindowPos(window, &windowX, &windowY);

            return true;
        }

        void Terminate() {
            if (window) {
                glfwDestroyWindow(window);
                window = nullptr;
            }
            glfwTerminate();
        }

        bool ShouldClose() const {
            return window && glfwWindowShouldClose(window);
        }

        void GetWindowSize(int& width, int& height) const {
            width = windowWidth;
            height = windowHeight;
        }

        int GetWindowWidth() const {
            return windowWidth;
        }

        int GetWindowHeight() const {
            return windowHeight;
        }

        // Nueva función para obtener la posición de la ventana en el escritorio
        void GetWindowPosition(int& x, int& y) const {
            x = windowX;
            y = windowY;
        }

        void PollEvents() {
            if (window) {
                glfwPollEvents();
            }
        }

        void SwapBuffers() {
            if (window) {
                glfwSwapBuffers(window);
            }
        }

        void SetupInputCallbacks() {
            glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                if (action == GLFW_PRESS) {
                    InputManager::Instance().SetKeyPressed(key, true);
                }
                else if (action == GLFW_RELEASE) {
                    InputManager::Instance().SetKeyPressed(key, false);
                }
                });

            glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
                double x, y;
                glfwGetCursorPos(window, &x, &y);

                if (action == GLFW_PRESS) {
                    InputManager::Instance().SetMousePressed(button, x, y, true);
                }
                else if (action == GLFW_RELEASE) {
                    InputManager::Instance().SetMousePressed(button, x, y, false);
                }
                });

            glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
                InputManager::Instance().SetMousePosition(xpos, ypos);
                });

            // Aquí añades el callback de posición de la ventana
            glfwSetWindowPosCallback(window, window_pos_callback);
        }

        GLFWwindow* GetWindow() const {
            return window;
        }

    private:
        WindowManager() : window(nullptr), windowWidth(0), windowHeight(0), windowX(0), windowY(0) {}

        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        ~WindowManager() {
            Terminate();
        }

        GLFWwindow* window;
        int windowWidth;
        int windowHeight;
        int windowX;  // Nueva variable para almacenar la posición X de la ventana
        int windowY;  // Nueva variable para almacenar la posición Y de la ventana

        static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
            WindowManager& instance = WindowManager::GetInstance();
            instance.windowWidth = width;
            instance.windowHeight = height;
            EventManager::OnWindowResizeEvent().trigger(width, height);
            glViewport(0, 0, width, height);
        }

        // Agregar este código a WindowManager
        static void window_pos_callback(GLFWwindow* window, int xpos, int ypos) {
            WindowManager& instance = WindowManager::GetInstance();
            instance.windowX = xpos;  // Actualizar las posiciones internas
            instance.windowY = ypos;
            EventManager::OnWindowMovedEvent().trigger(xpos, ypos);  // Lanzar un evento con las nuevas posiciones
        }
    };
}
