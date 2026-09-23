#include "HealthController.h"

// Add definition of your processing function here

void HealthController::health(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback
)const
{
    Json::Value body;
    body["status"] = "OK";

    auto response =
        HttpResponse::newHttpJsonResponse(body);

    callback(response);
}