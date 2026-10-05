#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <drogon/orm/DbClient.h>
#include <drogon/orm/Exception.h>

#include "models/Tasks.h"

class TaskService
{
public:
    using Task = drogon_model::sqlite3::Tasks;
    using SuccessCallback = std::function<void(Task)>;
    using ListCallback = std::function<void(std::vector<Task>)>;
    using ErrorCallback =
        std::function<void(const drogon::orm::DrogonDbException&)>;

    explicit TaskService(drogon::orm::DbClientPtr dbClient);

    void create(
        std::int64_t userId,
        std::string title,
        SuccessCallback onSuccess,
        ErrorCallback onError
    ) const;

    void listByUserId(
        std::int64_t userId,
        ListCallback onSuccess,
        ErrorCallback onError
    ) const;

private:
    drogon::orm::DbClientPtr dbClient_;
};
