#include "WorkerQueue.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

WorkerQueue::WorkerQueue(std::size_t maxPending)
    : maxPending_(maxPending)
{
    if(maxPending_ == 0)
    {
        throw std::invalid_argument("maxPending must be greater than 0");
    }

    worker_ = std::thread([this]()
    {
        run();
    });
}

WorkerQueue::~WorkerQueue()
{
    stop();
}

WorkerQueue::PostResult WorkerQueue::post(std::function<void()> task)
{
    if (!task)
    {
        return WorkerQueue::PostResult::InvalidTask;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (stopping_)
        {
            return WorkerQueue::PostResult::Stopped;
        }

        if (tasks_.size() >= maxPending_)
        {
            return WorkerQueue::PostResult::Full;
        }

        tasks_.push(std::move(task));
    }

    condition_.notify_one();
    return WorkerQueue::PostResult::Accepted;
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
