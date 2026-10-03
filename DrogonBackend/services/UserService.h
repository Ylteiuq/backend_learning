#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <drogon/orm/DbClient.h>
#include <drogon/orm/Exception.h>
#include "models/Users.h"

class UserService
{
public:
    using User = drogon_model::sqlite3::Users;
    using SuccessCallback = std::function<void(User)>;
    using UpdateCallback = std::function<void(std::size_t)>;
    using DeleteCallback = std::function<void(std::size_t)>;
    using ListCallback = std::function<void(std::vector<User>)>;
    using ErrorCallback =
        std::function<void(const drogon::orm::DrogonDbException&)>;

    explicit UserService(drogon::orm::DbClientPtr dbClient);

    void findById(
        std::int64_t id,
        SuccessCallback onSuccess,
        ErrorCallback onError
    ) const;

    void create(
        std::string name,
        std::int64_t age,
        SuccessCallback onSuccess,
        ErrorCallback onError
    ) const;

    void update(
        std::int64_t id,
        std::string name,
        std::int64_t age,
        UpdateCallback onSuccess,
        ErrorCallback onError
    ) const;

    void deleteById(
        std::int64_t id,
        DeleteCallback onSuccess,
        ErrorCallback onError
    ) const;

    void list(
        std::size_t page,
        std::size_t perPage,
        ListCallback onSuccess,
        ErrorCallback onError
    ) const;

private:
    drogon::orm::DbClientPtr dbClient_;
};
