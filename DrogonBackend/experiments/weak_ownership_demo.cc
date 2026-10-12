#include <iostream>
#include <memory>

struct Task;

struct User
{
    std::string name;
    std::vector<std::shared_ptr<Task>> tasks;

    ~User()
    {
        std::cout << name << '\n';
        std::cout << "User ending\n";
    }
};


struct Task
{
    std::string title;
    // std::shared_ptr<User> owner;
    std::weak_ptr<User> owner;

    ~Task()
    {
        std::cout << title << '\n';
        std::cout << "Task ending\n";
    }

    void printOwner() const;
};

void Task::printOwner() const
{
    auto user = owner.lock();
    if(user)
    {
        std::cout << user.get()->name;
    }
    else
    {
        std::cout << "Owner expired";
    }
}

std::shared_ptr<Task> savedTask;

int main()
{
    // {
    // auto p = std::make_shared<User>();
    // auto q = std::make_shared<Task>();
    // p->name = "Alice";
    // p->tasks.push_back(q);
    // q->title = "task1";
    // q->owner = p;

    // std::cout << p.use_count() << '\n' << q.use_count() << '\n';

    // std::cout << "scope ended";
    // }

    {
        auto p = std::make_shared<User>();
        auto q = std::make_shared<Task>();
        p->name = "Eric";
        p->tasks.push_back(q);
        savedTask = q;
        q->title = "Task2";
        q->owner =  p;

        q->printOwner();
    }
    savedTask->printOwner();
    savedTask.reset();
}
