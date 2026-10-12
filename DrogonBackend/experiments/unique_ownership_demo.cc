#include <iostream>
#include <memory>

class Resource
{
public:
    Resource()
    {
        std::cout << "struct begin\n";
    }

    ~Resource()
    {
        std::cout << "struct end\n";
    }
};

void consume(std::unique_ptr<Resource> resource)
{
    if(resource == nullptr)
    {
        std::cout << "resource is empty\n";
    }
    else
    {
        std::cout << "resource is true\n";
    }

    std::cout << "consume ending\n";
}

int main()
{
    auto p = std::make_unique<Resource>();
    std::unique_ptr<Resource>&& res = std::move(p);

    std::cout << "after binding: " << (p != nullptr) << '\n';

    consume(std::move(res));
}
