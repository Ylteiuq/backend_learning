#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class Counter
{
public:
    void increment();
    int value();
    std::mutex& mutex();

private:
    std::mutex mutex_;
    int value_{0};
};

void Counter::increment()
{
    int i = 0;
    while (i<10000)
    {
        ++value_;
        ++i;
    }
}

int Counter::value()
{
    return value_;
}

std::mutex& Counter::mutex()
{
    return mutex_;
}

int main()
{
    Counter counter;
    std::vector<std::thread> thread;

    for(int i = 0; i<4;++i)
    {
        thread.emplace_back([&counter]
        {
            std::lock_guard<std::mutex> lock(counter.mutex());
            counter.increment();
        });
    }

    for(auto& t : thread)
    {
        if(t.joinable())
        {
            t.join();
        }
    }
    std::cout << "counter: " << counter.value() << '\n';
}
