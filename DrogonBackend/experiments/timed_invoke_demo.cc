#include <iostream>
#include <memory>
#include <chrono>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <utility>
#include <string>

class ScopeTimer
{
public:
    explicit ScopeTimer(std::string label);
    ~ScopeTimer() noexcept;

private:
    using Clock = std::chrono::steady_clock;

    std::string label_;
    Clock::time_point start_;
};

ScopeTimer::ScopeTimer(std::string label)
    : label_(label), start_(Clock::now())
{
}

ScopeTimer::~ScopeTimer() noexcept
{
    auto time = std::chrono::duration<double,std::milli>(
        Clock::now() - start_).count();
    std::fprintf(stderr, "%s: %.3f ms\n", label_.c_str(), time);
}

template<typename F, typename... Args>
decltype(auto) timedInvoke(
    const char* label, F&& fn, Args&&... args)
{
    ScopeTimer scopeTimer(label);

    return std::invoke(std::forward<F>(fn), std::forward<Args>(args)...);
}

int main()
{
    struct Counter
    {
        int value = 0;

        void add(int n)
        {
            value += n;
        }
    };

    Counter counter;
    timedInvoke("counter.add", &Counter::add, counter, 5);
    std::cout << counter.value << '\n';
}