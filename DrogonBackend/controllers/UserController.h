#pragma once

#include <drogon/HttpController.h>

#include <functional>
#include <string>

class UserController : public drogon::HttpController<UserController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(UserController::getById, "/users/{1}", drogon::Get);
    ADD_METHOD_TO(UserController::createUser, "/users", drogon::Post);
    METHOD_LIST_END

    void getById(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        const std::string id
    );

    void createUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};
