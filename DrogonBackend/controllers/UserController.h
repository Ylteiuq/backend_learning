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
    ADD_METHOD_TO(UserController::listUsers, "/users", drogon::Get);
    ADD_METHOD_TO(UserController::updateUser, "/users/{1}", drogon::Put);
    ADD_METHOD_TO(UserController::deleteUser, "/users/{1}", drogon::Delete);
    ADD_METHOD_TO(UserController::createTask, "/users/{1}/tasks", drogon::Post);
    ADD_METHOD_TO(UserController::listTasks, "/users/{1}/tasks", drogon::Get);
    ADD_METHOD_TO(
        UserController::updateTaskCompleted,
        "/users/{1}/tasks/{2}",
        drogon::Patch);
    METHOD_LIST_END

    void getById(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void createUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void listUsers(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void updateUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void deleteUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void createTask(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void listTasks(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string userId
    );

    void updateTaskCompleted(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string userId,
        std::string taskId
    );
};
