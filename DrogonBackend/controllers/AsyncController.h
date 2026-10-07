#pragma once

#include <drogon/HttpController.h>

#include <functional>

class AsyncController
    : public drogon::HttpController<AsyncController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(
        AsyncController::delay,
        "/debug/delay",
        drogon::Get);
    METHOD_LIST_END

    void delay(
        const drogon::HttpRequestPtr &req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
