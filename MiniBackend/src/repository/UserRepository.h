#pragma once

#include "model/User.h"

#include <optional>
#include <sqlite3.h>


class UserRepository
{
public:
    UserRepository();
    ~UserRepository();

    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;

    User save(
        const User& user
    );

    std::optional<User> findById(
        int id
    );

private:
    sqlite3* db_;
};
