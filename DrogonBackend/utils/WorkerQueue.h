#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class WorkerQueue
{
public:
    WorkerQueue();
    ~WorkerQueue();

    WorkerQueue(const WorkerQueue &) = delete;
    WorkerQueue &operator=(const WorkerQueue &) = delete;

    bool post(std::function<void()> task);

    // Called by the owning thread; waits for accepted tasks to finish.
    void stop();

private:
    void run();

    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::function<void()>> tasks_;
    bool stopping_ = false;
    std::thread worker_;
};
