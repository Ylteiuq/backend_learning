#include "UserController.h"

#include <json/json.h>

#include <optional>
#include <string>
#include <utility>
#include <charconv>
#include <cstdint>
#include <system_error>

#include <drogon/orm/Mapper.h>
#include <trantor/utils/Logger.h>

#include "models/Users.h"

namespace
{
    std::optional<std::string> validateCreateUserBody(
        const Json::Value& body
    )
    {
        if(!body.isObject())
        {
            return "request body must be a JSON object";
        }

        if(!body.isMember("name"))
        {
            return "name is required";
        }

        if(!body.isMember("age"))
        {
            return "age is required";
        }

        if(!body["name"].isString())
        {
            return "name must be a string";
        }

        if(!body["age"].isInt())
        {
            return "age must be an integer";
        }

        return std::nullopt;
    }

    drogon::HttpResponsePtr makeJsonResponse(
        Json::Value body,
        drogon::HttpStatusCode status
    )
    {
        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
                std::move(body)
            );

        response->setStatusCode(status);
        return response;
    }

    drogon::HttpResponsePtr makeErrorResponse(
        const std::string& message,
        drogon::HttpStatusCode status
    )
    {
        Json::Value body;
        body["error"] = message;

        return makeJsonResponse(
            std::move(body),
            status
        );
    }
}

void UserController::getById(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    const std::string id
)
{
    std::int64_t parseId = 0;

    const auto [end,error] = std::from_chars(
        id.data(),
        id.data() + id.size(),
        parseId
    );

    if(error != std::errc{} ||
        end != id.data() + id.size() ||
        parseId <= 0)
    {
        callback(
            makeErrorResponse(
                "invalid user id",
                drogon::k400BadRequest
            )
        );
        return;
    }

    using UserModel = drogon_model::sqlite3::Users;

    auto dbClient = drogon::app().getDbClient("default");
    drogon::orm::Mapper<UserModel> mapper(dbClient);

    auto databaseErrorCallback = callback;

    mapper.findByPrimaryKey(
        parseId,
        [callback = std::move(callback)](UserModel&& user){
            callback(
                makeJsonResponse(
                    user.toJson(),
                    drogon::k200OK
                )
            );
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException& error
        ){
            const auto* unexpectedRows =
                dynamic_cast<const drogon::orm::UnexpectedRows*>(
                    &error.base()
                );
            
            if(unexpectedRows != nullptr)
            {
                callback(
                    makeErrorResponse(
                        "User not found",
                        drogon::k404NotFound
                    )
                );
                return;
            }

            LOG_ERROR << "Failed to find user: "
                << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError
                )
            );
        }
    );
}

void UserController::createUser(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
)
{
    const auto json = req->getJsonObject();

    if(!json)
    {
        callback(
            makeErrorResponse(
                "invalid JSON or Content-Type must be application/json",
                drogon::k400BadRequest
            )
        );
        return;
    }

    if(const auto validationError = validateCreateUserBody(*json))
    {
        callback(
            makeErrorResponse(
                *validationError,
                drogon::k400BadRequest
            )
        );
        return;
    }

    const std::string name = (*json)["name"].asString();
    const int age = (*json)["age"].asInt();

    using UserModel = drogon_model::sqlite3::Users;

    UserModel user;
    user.setName(name);
    user.setAge(age);

    auto dbClient = drogon::app().getDbClient("default");
    drogon::orm::Mapper<UserModel> mapper(dbClient);

    auto databaseErrorCallback = callback;

    mapper.insert(
        user,
        [callback = std::move(callback)](UserModel insertedUser){
            callback(
                makeJsonResponse(
                    insertedUser.toJson(),
                    drogon::k201Created
                )
            );
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException& error
        ){
            LOG_ERROR << "Failed to create user"
                << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError
                )
            );
        }
    );
}
