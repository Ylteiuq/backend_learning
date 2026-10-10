#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <cstddef>

class WorkerQueue
{
public:
    ~WorkerQueue();

    WorkerQueue(const WorkerQueue &) = delete;
    WorkerQueue &operator=(const WorkerQueue &) = delete;

    // Called by the owning thread; waits for accepted tasks to finish.
    void stop();

    enum class PostResult
    {
        Accepted,
        Full,
        Stopped,
        InvalidTask
    };

    explicit WorkerQueue(std::size_t maxPending = 32);

    PostResult post(std::function<void()> task);
private:
    void run();

    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::function<void()>> tasks_;
    bool stopping_ = false;
    std::thread worker_;
    const std::size_t maxPending_;
};
