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
    ADD_METHOD_TO(
        AsyncController::countPrimes,
        "/debug/primes",
        drogon::Get);
    ADD_METHOD_TO(
        AsyncController::getComputeStats,
        "/debug/compute/stats",
        drogon::Get);
    METHOD_LIST_END

    void delay(
        const drogon::HttpRequestPtr &req,
        std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void countPrimes(
        const drogon::HttpRequestPtr &req,
        std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getComputeStats(
        const drogon::HttpRequestPtr &req,
        std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
