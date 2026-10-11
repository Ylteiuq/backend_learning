#include <drogon/orm/DbClient.h>
#include <drogon/orm/Mapper.h>

#include "models/Tasks.h"

#include <chrono>
#include <cstddef>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3)
    {
        std::cerr << "Usage: TransactionDemo <database-file> [--fail]\n";
        return 1;
    }

    bool failMode = false;

    if (argc == 3)
    {
        if (std::string(argv[2]) != "--fail")
        {
            std::cerr << "Invalid argument\n";
            return 1;
        }
        failMode = true;
    }

    try
    {
        using Task = drogon_model::sqlite3::Tasks;

        auto db = drogon::orm::DbClient::newSqlite3Client(
            std::string("filename=") + argv[1], 1);

        db->setTimeout(5.0);
        db->execSqlSync("PRAGMA foreign_keys = ON;");

        auto commitPromise = std::make_shared<std::promise<bool>>();
        auto commitFuture = commitPromise->get_future();

        {
            auto transaction = db->newTransaction(
                [commitPromise](bool success)
                {
                    commitPromise->set_value(success);
                });

            try
            {
                drogon::orm::Mapper<Task> mapper(transaction);

                std::vector<std::string> titles = {
                    "First transaction task",
                    "Second transaction task"
                };

                for (std::size_t i = 0; i < titles.size(); ++i)
                {
                    Task task;

                    task.setUserId(1);
                    task.setCompleted(0);

                    if (!(failMode && i == 1))
                    {
                        task.setTitle(titles[i]);
                    }

                    mapper.insert(task);

                    std::cout << "Inserted task " << i + 1 << '\n';
                }
            }
            catch (...)
            {
                transaction->rollback();

                throw;
            }
        }

        if (commitFuture.wait_for(std::chrono::seconds(5)) !=
            std::future_status::ready)
        {
            std::cerr << "Timed out waiting for transaction commit\n";
            return 1;
        }

        const bool success = commitFuture.get();

        if (!success)
        {
            std::cerr << "Transaction commit failed\n";
            return 1;
        }

        std::cerr << "Transaction committed successfully\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "unknown error\n";
        return 1;
    }

    return 0;
}
