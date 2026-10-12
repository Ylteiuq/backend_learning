#include <iostream>
#include <memory>

struct Resource
{
    Resource()
    {
        std::cout << "Resource constructed\n";
    }

    ~Resource()
    {
        std::cout << "Resource destroyed\n";
    }
};

int main()
{
    auto p = std::make_shared<Resource>();

    auto q = p;

    std::cout << p.use_count() << '\n' << q.use_count() << '\n';

    std::cout << "flag p" << '\n';
    p.reset();
    std::cout << "p end" << '\n';
    std::cout << p.use_count() << '\n' << q.use_count() << '\n';
    std::cout << "q beginend" << '\n';
    q.reset();
    std::cout << "q end" << '\n';
    std::cout << p.use_count() << '\n' << q.use_count() << '\n';

    return 0;
}
