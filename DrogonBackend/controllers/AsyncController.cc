#include "AsyncController.h"

#include <drogon/drogon.h>

#include <string>
#include <utility>

namespace
{
    drogon::HttpResponsePtr makeJsonResponse(
        Json::Value body,
        drogon::HttpStatusCode status)
    {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            body);

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
}

void AsyncController::delay(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)> &&callback)
{
    std::string message = req->getParameter("message");

    if(message.empty())
    {
        callback(
            makeErrorResponse(
                "invalid message",
                drogon::k400BadRequest
            )
        );
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
                    body,
                    drogon::k200OK
                )
            );
        }
    );
}
