#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

// BackgroundTaskManager class
class BackgroundTaskManager {
public:
    struct Task {
        std::function<void()> startJob;
        std::function<void()> progressJob;
        std::function<void()> endJob;

        // Constructor predeterminado
        Task() = default;

        Task(const std::function<void()>& start, const std::function<void()>& progress, const std::function<void()>& end)
            : startJob(start), progressJob(progress), endJob(end) {}
    };

    // Singleton access
    static BackgroundTaskManager& GetInstance() {
        static BackgroundTaskManager instance;
        return instance;
    }

    // Add task to the queue
    void AddTask(Task task) {
        {
            std::lock_guard<std::mutex> lock(taskMutex);
            taskQueue.push(task);
        }
        taskCondition.notify_one(); // Notify the worker thread
    }

    // Start the background worker thread
    void Start() {
        workerThread = std::thread([this]() {
            while (true) {
                Task currentTask;

                {
                    std::unique_lock<std::mutex> lock(taskMutex);

                    // Wait until a task is available or the worker stops
                    taskCondition.wait(lock, [this]() { return !taskQueue.empty() || stopWorker; });

                    if (stopWorker && taskQueue.empty())
                        break;

                    currentTask = taskQueue.front();
                    taskQueue.pop();
                }

                // Execute start job
                if (currentTask.startJob) {
                    currentTask.startJob();
                }

                // Execute progress job, if necessary
                if (currentTask.progressJob) {
                    currentTask.progressJob();
                }

                // Execute end job in the main thread using MainThreadTaskManager
                if (currentTask.endJob) {
                    MainThreadTaskManager::GetInstance().AddTask(currentTask.endJob);
                }
            }
            });
    }

    // Stop the background worker thread
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(taskMutex);
            stopWorker = true;
        }
        taskCondition.notify_all();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

private:
    BackgroundTaskManager() : stopWorker(false) {}

    std::queue<Task> taskQueue;
    std::mutex taskMutex;
    std::condition_variable taskCondition;
    bool stopWorker;
    std::thread workerThread;
};

