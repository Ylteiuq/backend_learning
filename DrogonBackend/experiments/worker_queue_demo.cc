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
}

int main()
{
    try
    {
        int total = 0;
        std::thread::id workerThread;
        std::vector<int> executionOrder;
        bool acceptsAfterStop = false;

        {
            WorkerQueue queue;

            check(!queue.post({}), "Empty tasks must be rejected");

            check(queue.post([&total, &workerThread, &executionOrder]()
            {
                workerThread = std::this_thread::get_id();
                executionOrder.push_back(1);
                total += 10;
            }), "First task must be accepted");

            check(queue.post([]()
            {
                throw std::runtime_error("demo error");
            }), "Standard exception task must be accepted");

            check(queue.post([]()
            {
                throw 42;
            }), "Non-standard exception task must be accepted");

            check(queue.post([&total, &executionOrder]()
            {
                executionOrder.push_back(2);
                total += 20;
            }), "Task after exceptions must be accepted");

            queue.stop();
            queue.stop();

            acceptsAfterStop = queue.post([]() {});
        }

        check(!acceptsAfterStop, "Tasks after stop must be rejected");
        check(total == 30, "Accepted tasks must finish despite exceptions");
        check(executionOrder == std::vector<int>{1, 2},
              "Tasks must execute in submission order");
        check(workerThread != std::this_thread::get_id(),
              "Tasks must execute on the worker thread");

        std::cout << std::boolalpha
                  << "accepts_after_stop=" << acceptsAfterStop << '\n'
                  << "total=" << total << '\n'
                  << "PASS: task order, exceptions, worker thread, repeated stop\n";

        int destructorTotal = 0;

        {
            WorkerQueue queue;

            check(queue.post([&destructorTotal]() { destructorTotal += 7; }),
                  "First destructor task must be accepted");
            check(queue.post([&destructorTotal]() { destructorTotal += 8; }),
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
