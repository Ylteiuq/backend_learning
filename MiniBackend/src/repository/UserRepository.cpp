#include "repository/UserRepository.h"

#include <memory>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace
{
    struct StatementDeleter
    {
        void operator()(sqlite3_stmt* statement) const noexcept
        {
            if(statement != nullptr)
            {
                sqlite3_finalize(statement);
            }
        }
    };

    using StatementPtr =
        std::unique_ptr<sqlite3_stmt, StatementDeleter>;
}

UserRepository::UserRepository()
    : db_(nullptr)
{
    int result =
        sqlite3_open(
            "minibackend.db",
            &db_
        );

    if(result != SQLITE_OK)
    {
        const std::string message =
            db_ != nullptr
                ? sqlite3_errmsg(db_)
                : "failed to open database";

        if(db_ != nullptr)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }

        throw std::runtime_error(message);
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            age INTEGER NOT NULL
        );
    )";

    char* errorMessage = nullptr;

    result =
        sqlite3_exec(
            db_,
            sql,
            nullptr,
            nullptr,
            &errorMessage
        );

    if (result != SQLITE_OK)
    {
        std::string message =
            errorMessage
                ? errorMessage
                : "failed to create users table";
        sqlite3_free(errorMessage);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error(message);
    }
}

UserRepository::~UserRepository()
{
    if(db_ != nullptr)
    {
        sqlite3_close(db_);
    }
}

User UserRepository::save(
    const User& user
)
{
    const char* sql =
        "INSERT INTO users (name, age) VALUES (?, ?);";

    sqlite3_stmt* rawStatement = nullptr;

    int result =
        sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &rawStatement,
            nullptr
        );

    StatementPtr statement(rawStatement);

    if(result != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    if(sqlite3_bind_text(
        statement.get(),
        1,
        user.name.c_str(),
        -1,
        SQLITE_TRANSIENT
    ) != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    if(sqlite3_bind_int(
        statement.get(),
        2,
        user.age
    ) != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    result =
        sqlite3_step(statement.get());

    if(result != SQLITE_DONE)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    User savedUser = user;

    savedUser.id =
        static_cast<int>(
            sqlite3_last_insert_rowid(db_)
        );

    return savedUser;
}

std::optional<User> UserRepository::findById(
    int id
)
{
    const char* sql =
        "SELECT id, name, age FROM users WHERE id = ?;";

    sqlite3_stmt* rawStatement = nullptr;

    int result =
        sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &rawStatement,
            nullptr
        );

    StatementPtr statement(rawStatement);

    if(result != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    result =
        sqlite3_bind_int(
            statement.get(),
            1,
            id
        );

    if(result != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    result =
        sqlite3_step(statement.get());

    if(result == SQLITE_DONE)
    {
        return std::nullopt;
    }

    if(result != SQLITE_ROW)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    User user{};
    user.id =
        sqlite3_column_int(
            statement.get(),
            0
        );
    user.name =
        reinterpret_cast<const char*>(
            sqlite3_column_text(
                statement.get(),
                1
            )
        );
    user.age =
        sqlite3_column_int(
            statement.get(),
            2
        );

    return user;
}
