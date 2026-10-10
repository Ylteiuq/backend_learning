#include "utils/WorkerQueue.h"

#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{
    void check(bool condition, const char *message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    std::string postResultResponse(WorkerQueue::PostResult result)
    {
        switch (result)
        {
        case WorkerQueue::PostResult::Full:
            return "compute queue is full";
        case WorkerQueue::PostResult::InvalidTask:
            return "invalid compute task";
        case WorkerQueue::PostResult::Stopped:
            return "compute queue is stopped";

        default:
            return "unknown error";
        }
    }
}

int main()
{
    try
    {
        int total = 0;
        std::thread::id workerThread;
        std::vector<int> executionOrder;
        WorkerQueue::PostResult acceptsAfterStop;

        {
            WorkerQueue queue;

            check(queue.post({}) == WorkerQueue::PostResult::InvalidTask, "Empty tasks must be rejected");

            check(queue.post([&total, &workerThread, &executionOrder]()
            {
                workerThread = std::this_thread::get_id();
                executionOrder.push_back(1);
                total += 10;
            }) == WorkerQueue::PostResult::Accepted, "First task must be accepted");

            check(queue.post([]()
            {
                throw std::runtime_error("demo error");
            }) == WorkerQueue::PostResult::Accepted, "Standard exception task must be accepted");

            check(queue.post([]()
            {
                throw 42;
            }) == WorkerQueue::PostResult::Accepted, "Non-standard exception task must be accepted");

            check(queue.post([&total, &executionOrder]()
            {
                executionOrder.push_back(2);
                total += 20;
            }) == WorkerQueue::PostResult::Accepted, "Task after exceptions must be accepted");

            queue.stop();
            queue.stop();

            acceptsAfterStop = queue.post([]() {});
        }

        check(acceptsAfterStop == WorkerQueue::PostResult::Stopped, "Tasks after stop must be rejected");
        check(total == 30, "Accepted tasks must finish despite exceptions");
        check(executionOrder == std::vector<int>{1, 2},
              "Tasks must execute in submission order");
        check(workerThread != std::this_thread::get_id(),
              "Tasks must execute on the worker thread");

        std::cout << std::boolalpha
                  << "accepts_after_stop=" << postResultResponse(acceptsAfterStop) << '\n'
                  << "total=" << total << '\n'
                  << "PASS: task order, exceptions, worker thread, repeated stop\n";

        int destructorTotal = 0;

        {
            WorkerQueue queue;

            check(queue.post([&destructorTotal]() { destructorTotal += 7; }) == WorkerQueue::PostResult::Accepted,
                  "First destructor task must be accepted");
            check(queue.post([&destructorTotal]() { destructorTotal += 8; }) == WorkerQueue::PostResult::Accepted,
                  "Second destructor task must be accepted");
        }

        check(destructorTotal == 15,
              "Destruction must finish accepted tasks and join the worker");
        std::cout << "PASS: destructor drains tasks, total="
                  << destructorTotal << '\n';

        {
            WorkerQueue queue;
            queue.stop();
        }

        std::cout << "PASS: idle worker stops normally\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
