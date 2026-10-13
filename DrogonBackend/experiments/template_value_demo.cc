#include <iostream>
#include <utility>

struct Trace
{
    Trace();
    Trace(const Trace&);
    Trace(Trace&&) noexcept;
};

void consume(Trace&)
{
    std::cout << "consume L\n";
}

void consume(Trace&&)
{
    std::cout << "consume R\n";
}

void consume(std::unique_ptr<int>& value)
{
    std::cout << *value << '\n';
}

void consume(std::unique_ptr<int>&& value)
{
    const auto own = std::move(value);
    std::cout << *own << '\n';
}

template<typename T>
void relay(T&& value)
{
    consume(value);
    consume(std::move(value));
    consume(std::forward<T>(value));
}

int main()
{
    Trace temp;
    consume(temp);
    consume(std::move(temp));
    relay(temp);
}
