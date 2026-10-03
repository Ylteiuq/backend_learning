#include "UserService.h"

#include <drogon/orm/Mapper.h>

#include <utility>

UserService::UserService(drogon::orm::DbClientPtr dbClient)
    : dbClient_(std::move(dbClient))
{
}

void UserService::findById(
    std::int64_t id,
    SuccessCallback onSuccess,
    ErrorCallback onError
) const
{
    drogon::orm::Mapper<User> mapper(dbClient_);

    mapper.findByPrimaryKey(
        id,
        std::move(onSuccess),
        std::move(onError)
    );
}

void UserService::create(
    std::string name,
    std::int64_t age,
    SuccessCallback onSuccess,
    ErrorCallback onError
) const
{
    User user;
    user.setName(std::move(name));
    user.setAge(age);

    drogon::orm::Mapper<User> mapper(dbClient_);

    mapper.insert(
        user,
        std::move(onSuccess),
        std::move(onError)
    );
}

void UserService::update(
    std::int64_t id,
    std::string name,
    std::int64_t age,
    UpdateCallback onSuccess,
    ErrorCallback onError
) const
{
    User user;
    user.setId(id);
    user.setName(std::move(name));
    user.setAge(age);

    drogon::orm::Mapper<User> mapper(dbClient_);

    mapper.update(
        user,
        std::move(onSuccess),
        std::move(onError)
    );
}

void UserService::deleteById(
    std::int64_t id,
    UserService::DeleteCallback onSuccess,
    UserService::ErrorCallback onError
) const
{
    drogon::orm::Mapper<User> mapper(dbClient_);

    mapper.deleteByPrimaryKey(
        id,
        std::move(onSuccess),
        std::move(onError)
    );
}

void UserService::list(
    std::size_t page,
    std::size_t perPage,
    ListCallback onSuccess,
    ErrorCallback onError
) const
{
    drogon::orm::Mapper<User> mapper(dbClient_);

    mapper.orderBy(User::Cols::_id)
        .paginate(page, perPage)
        .findAll(std::move(onSuccess),std::move(onError));
}
