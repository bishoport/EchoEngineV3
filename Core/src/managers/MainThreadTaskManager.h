#include <queue>
#include <functional>
#include <mutex>

class MainThreadTaskManager {
public:
    // Singleton access
    static MainThreadTaskManager& GetInstance() {
        static MainThreadTaskManager instance;
        return instance;
    }

    // Add task to the queue
    void AddTask(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(task);
    }

    // Execute tasks in the main thread
    void ExecuteTasks() {
        std::queue<std::function<void()>> tasksCopy;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasksCopy.swap(taskQueue);
        }
        while (!tasksCopy.empty()) {
            tasksCopy.front()();
            tasksCopy.pop();
        }
    }

private:
    MainThreadTaskManager() {}

    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
};


