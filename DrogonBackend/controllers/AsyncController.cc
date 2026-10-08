#include "AsyncController.h"

#include <drogon/drogon.h>
#include <trantor/utils/ConcurrentTaskQueue.h>

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <system_error>
#include <utility>

namespace
{
    constexpr int kMaxPrimeLimit = 5'000'000;

    drogon::HttpResponsePtr makeJsonResponse(
        Json::Value body,
        drogon::HttpStatusCode status)
    {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            std::move(body));

        response->setStatusCode(status);

        return response;
    }

    drogon::HttpResponsePtr makeErrorResponse(
        const std::string &message,
        drogon::HttpStatusCode status)
    {
        Json::Value body;

        body["error"] = message;

        return makeJsonResponse(
            std::move(body),
            status);
    }

    bool isPrime(int value)
    {
        if (value < 2)
        {
            return false;
        }

        for (int divisor = 2;
            divisor <= value / divisor;
            ++divisor)
        {
            if (value % divisor == 0)
            {
                return false;
            }
        }

        return true;
    }

    int calculatePrimeCount(int limit)
    {
        int count = 0;

        for (int value = 2; value <= limit; ++value)
        {
            if (isPrime(value))
            {
                ++count;
            }
        }

        return count;
    }

    trantor::ConcurrentTaskQueue &computeQueue()
    {
        static trantor::ConcurrentTaskQueue queue(
            1,
            "compute");

        return queue;
    }

    std::optional<std::int64_t> parseInt64(
        const std::string &value)
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        std::int64_t parsedValue = 0;
        const auto [end, error] = std::from_chars(
            value.data(),
            value.data() + value.size(),
            parsedValue);

        if (error != std::errc{} || end != value.data() + value.size())
        {
            return std::nullopt;
        }

        return parsedValue;
    }
}

void AsyncController::delay(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    std::string message = req->getParameter("message");

    if (message.empty())
    {
        callback(
            makeErrorResponse(
                "invalid message",
                drogon::k400BadRequest));
        return;
    }

    drogon::app().getLoop()->runAfter(
        3.0,
        [callback = std::move(callback),
         message = std::move(message)]()
        {
            Json::Value body;
            body["message"] = message;
            body["delay_seconds"] = 3.0;

            callback(
                makeJsonResponse(
                    std::move(body),
                    drogon::k200OK));
        });
}

void AsyncController::countPrimes(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    const auto &text = req->getParameter("limit");

    const auto parsedLimit = parseInt64(text);

    if (!parsedLimit || *parsedLimit < 2 || *parsedLimit > kMaxPrimeLimit)
    {
        callback(
            makeErrorResponse(
                "limit must be an integer between 2 and 5000000",
                drogon::k400BadRequest));
        return;
    }

    const int limit = static_cast<int>(*parsedLimit);

    LOG_INFO << "A: submitting prime calculation";

    computeQueue().runTaskInQueue(
        [limit, callback = std::move(callback)]()
        {
            LOG_INFO << "B: prime calculation started";

            const int count = calculatePrimeCount(limit);

            Json::Value body;
            body["limit"] = limit;
            body["count"] = count;

            callback(
                makeJsonResponse(
                    std::move(body),
                    drogon::k200OK));
        });

    LOG_INFO << "C: calculation submitted";
}
