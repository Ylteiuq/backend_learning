#include "UserController.h"

#include <json/json.h>

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <drogon/orm/Mapper.h>
#include <trantor/utils/Logger.h>

#include "models/Users.h"
#include "services/UserService.h"

namespace
{
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

    std::optional<std::string> validateCreateUserBody(
        const Json::Value &body)
    {
        if (!body.isObject())
        {
            return "request body must be a JSON object";
        }

        if (!body.isMember("name"))
        {
            return "name is required";
        }

        if (!body.isMember("age"))
        {
            return "age is required";
        }

        if (!body["name"].isString())
        {
            return "name must be a string";
        }

        if (!body["age"].isInt())
        {
            return "age must be an integer";
        }

        return std::nullopt;
    }

    drogon::HttpResponsePtr makeJsonResponse(
        Json::Value body,
        drogon::HttpStatusCode status)
    {
        auto response =
            drogon::HttpResponse::newHttpJsonResponse(
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
}

void UserController::getById(
    const drogon::HttpRequestPtr &,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    std::string id)
{
    const auto parsedId = parseInt64(id);
    if (!parsedId || *parsedId <= 0)
    {
        callback(
            makeErrorResponse(
                "invalid user id",
                drogon::k400BadRequest));
        return;
    }

    using UserModel = drogon_model::sqlite3::Users;

    UserService service(drogon::app().getDbClient("default"));

    auto databaseErrorCallback = callback;

    service.findById(
        *parsedId,
        [callback = std::move(callback)](UserModel user)
        {
            callback(
                makeJsonResponse(
                    user.toJson(),
                    drogon::k200OK));
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException &error)
        {
            const auto *unexpectedRows =
                dynamic_cast<const drogon::orm::UnexpectedRows *>(
                    &error.base());

            if (unexpectedRows != nullptr)
            {
                callback(
                    makeErrorResponse(
                        "User not found",
                        drogon::k404NotFound));
                return;
            }

            LOG_ERROR << "Failed to find user: "
                      << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError));
        });
}

void UserController::createUser(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    const auto json = req->getJsonObject();

    if (!json)
    {
        callback(
            makeErrorResponse(
                "invalid JSON or Content-Type must be application/json",
                drogon::k400BadRequest));
        return;
    }

    if (const auto validationError = validateCreateUserBody(*json))
    {
        callback(
            makeErrorResponse(
                *validationError,
                drogon::k400BadRequest));
        return;
    }

    const std::string name = (*json)["name"].asString();
    const int age = (*json)["age"].asInt();

    using UserModel = drogon_model::sqlite3::Users;

    UserService service(drogon::app().getDbClient("default"));

    auto databaseErrorCallback = callback;

    service.create(
        name,
        age,
        [callback = std::move(callback)](UserModel insertedUser)
        {
            callback(
                makeJsonResponse(
                    insertedUser.toJson(),
                    drogon::k201Created));
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException &error)
        {
            LOG_ERROR << "Failed to create user: "
                      << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError));
        });
}

void UserController::listUsers(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    const auto &pageParameter = req->getParameter("page");
    const auto &perPageParameter = req->getParameter("per_page");

    const auto page = pageParameter.empty()
                          ? std::optional<std::int64_t>{1}
                          : parseInt64(pageParameter);
    if (!page || *page <= 0)
    {
        callback(
            makeErrorResponse(
                "page must be a positive integer",
                drogon::k400BadRequest));
        return;
    }

    const auto perPage = perPageParameter.empty()
                             ? std::optional<std::int64_t>{10}
                             : parseInt64(perPageParameter);
    if (!perPage || *perPage <= 0 || *perPage > 100)
    {
        callback(
            makeErrorResponse(
                "per_page must be an integer between 1 and 100",
                drogon::k400BadRequest));
        return;
    }

    const auto pageNumber = static_cast<std::uint64_t>(*page);
    const auto perPageNumber = static_cast<std::uint64_t>(*perPage);
    const auto maxSize = static_cast<std::uint64_t>(
        std::numeric_limits<std::size_t>::max());
    const auto maxDatabaseOffset = static_cast<std::uint64_t>(
        std::numeric_limits<std::int64_t>::max());
    if (pageNumber > maxSize ||
        pageNumber - 1 > maxSize / perPageNumber ||
        pageNumber - 1 > maxDatabaseOffset / perPageNumber)
    {
        callback(
            makeErrorResponse(
                "page is too large",
                drogon::k400BadRequest));
        return;
    }

    using UserModel = drogon_model::sqlite3::Users;

    UserService service(drogon::app().getDbClient("default"));

    auto databaseErrorCallback = callback;
    const auto pageValue = *page;
    const auto perPageValue = *perPage;

    service.list(
        pageValue,
        perPageValue,
        [callback = std::move(callback), pageValue, perPageValue](
            std::vector<UserModel> users)
        {
            Json::Value userJson(Json::arrayValue);
            for (const auto &user : users)
            {
                userJson.append(user.toJson());
            }

            Json::Value body;
            body["users"] = std::move(userJson);
            body["page"] = static_cast<Json::Int64>(pageValue);
            body["per_page"] = static_cast<Json::Int64>(perPageValue);

            callback(
                makeJsonResponse(
                    std::move(body),
                    drogon::k200OK));
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException &error)
        {
            LOG_ERROR << "Failed to list users: "
                      << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError));
        });
}

void UserController::updateUser(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    std::string id)
{
    const auto parsedId = parseInt64(id);
    if (!parsedId || *parsedId <= 0)
    {
        callback(
            makeErrorResponse(
                "invalid user id",
                drogon::k400BadRequest));
        return;
    }

    const auto json = req->getJsonObject();

    if (!json)
    {
        callback(
            makeErrorResponse(
                "invalid JSON or Content-Type must be application/json",
                drogon::k400BadRequest));
        return;
    }

    if (const auto validationError = validateCreateUserBody(*json))
    {
        callback(
            makeErrorResponse(
                *validationError,
                drogon::k400BadRequest));
        return;
    }

    UserService service(drogon::app().getDbClient("default"));

    auto databaseErrorCallback = callback;

    service.update(
        *parsedId,
        (*json)["name"].asString(),
        (*json)["age"].asInt(),
        [callback = std::move(callback)](
            std::size_t affectedRows)
        {
            if (affectedRows == 0)
            {
                callback(
                    makeErrorResponse(
                        "User not found",
                        drogon::k404NotFound));
                return;
            }

            auto response = drogon::HttpResponse::newHttpResponse();
            response->setStatusCode(drogon::k204NoContent);
            callback(response);
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException &error)
        {
            LOG_ERROR << "Failed to update user: "
                      << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError));
        });
}

void UserController::deleteUser(
    const drogon::HttpRequestPtr &,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    std::string id)
{
    const auto parsedId = parseInt64(id);

    if (!parsedId || *parsedId <= 0)
    {
        callback(
            makeErrorResponse(
                "invalid user id",
                drogon::k400BadRequest));
        return;
    }

    using UserModel = drogon_model::sqlite3::Users;

    UserService service(drogon::app().getDbClient("default"));

    auto databaseErrorCallback = callback;

    service.deleteById(
        *parsedId,
        [callback = std::move(callback)](const std::size_t affectedRows)
        {
            if (affectedRows == 0)
            {
                callback(
                    makeErrorResponse(
                        "User not found",
                        drogon::k404NotFound));
                return;
            }

            auto response = drogon::HttpResponse::newHttpResponse();
            response->setStatusCode(drogon::k204NoContent);
            callback(response);
        },
        [callback = std::move(databaseErrorCallback)](
            const drogon::orm::DrogonDbException &error)
        {
            LOG_ERROR << "Failed to delete user: "
                      << error.base().what();

            callback(
                makeErrorResponse(
                    "Database error",
                    drogon::k500InternalServerError));
        });
}
