#include "WorkerQueue.h"

#include <exception>
#include <iostream>
#include <utility>

WorkerQueue::WorkerQueue()
{
    worker_ = std::thread([this]()
    {
        run();
    });
}

WorkerQueue::~WorkerQueue()
{
    stop();
}

bool WorkerQueue::post(std::function<void()> task)
{
    if (!task)
    {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (stopping_)
        {
            return false;
        }

        tasks_.push(std::move(task));
    }

    condition_.notify_one();
    return true;
}

void WorkerQueue::run()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            condition_.wait(lock, [this]()
            {
                return stopping_ || !tasks_.empty();
            });

            if (stopping_ && tasks_.empty())
            {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        try
        {
            task();
        }
        catch (const std::exception &error)
        {
            std::cerr << "WorkerQueue task failed: "
                      << error.what() << '\n';
        }
        catch (...)
        {
            std::cerr << "WorkerQueue task failed: unknown exception\n";
        }
    }
}

void WorkerQueue::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        stopping_ = true;
    }

    condition_.notify_all();

    if (worker_.joinable())
    {
        worker_.join();
    }
}
