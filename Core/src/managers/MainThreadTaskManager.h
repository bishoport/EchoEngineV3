#include <queue>
#include <functional>
#include <mutex>

class MainThreadTaskManager {
public:
    // Obtener la instancia singleton de la clase
    static MainThreadTaskManager& GetInstance() {
        static MainThreadTaskManager instance;
        return instance;
    }

    // Agregar tarea a la cola
    void AddTask(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(task);
    }

    // Ejecutar todas las tareas pendientes en el hilo principal
    void ExecuteTasks() {
        std::queue<std::function<void()>> tasksCopy;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasksCopy.swap(taskQueue);  // Mover tareas a una cola temporal para procesarlas
        }
        while (!tasksCopy.empty()) {
            tasksCopy.front()();  // Ejecutar la tarea
            tasksCopy.pop();
        }
    }

private:
    // Constructor privado para el patrón singleton
    MainThreadTaskManager() {}

    // Prohibir la copia o asignación
    MainThreadTaskManager(const MainThreadTaskManager&) = delete;
    MainThreadTaskManager& operator=(const MainThreadTaskManager&) = delete;

    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
};

