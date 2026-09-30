#define DROGON_TEST_MAIN

#include <drogon/drogon.h>
#include <drogon/drogon_test.h>

#include <future>
#include <string>
#include <thread>

DROGON_TEST(errorPostTest)
{
    using namespace drogon;

    auto client = HttpClient::newHttpClient("http://127.0.0.1:5555");

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setMethod(Post);
        req->setContentTypeCode(CT_APPLICATION_JSON);
        req->setBody("not-json");

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k400BadRequest);
            }
        );
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setMethod(Post);
        req->setContentTypeCode(CT_APPLICATION_JSON);
        req->setBody(R"({"name":"Alice"})");

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k400BadRequest);
            }
        );
    }
}

DROGON_TEST(errorGetTest)
{
    using namespace drogon;

    auto client = HttpClient::newHttpClient("http://127.0.0.1:5555");

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users/ABC");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k400BadRequest);
            }
        );
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users/7834719387");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k404NotFound);
            }
        );
    }
}

DROGON_TEST(createAndGetTest)
{
    using namespace drogon;

    auto client = HttpClient::newHttpClient("http://127.0.0.1:5555");

    HttpRequestPtr createRequest = HttpRequest::newHttpRequest();
    createRequest->setPath("/users");
    createRequest->setMethod(Post);
    createRequest->setContentTypeCode(CT_APPLICATION_JSON);
    createRequest->setBody(R"({"name":"Alice","age":30})");

    client->sendRequest(
        createRequest,
        [TEST_CTX, client](ReqResult result, const HttpResponsePtr& resp)
        {
            REQUIRE(result == ReqResult::Ok);
            REQUIRE(resp != nullptr);
            REQUIRE(resp->getStatusCode() == k201Created);

            const auto createdUserJson = resp->getJsonObject();
            REQUIRE(createdUserJson != nullptr);
            CHECK((*createdUserJson)["name"] == "Alice");
            CHECK((*createdUserJson)["age"] == 30);

            const auto id = (*createdUserJson)["id"].asInt64();
            HttpRequestPtr getRequest = HttpRequest::newHttpRequest();
            getRequest->setPath("/users/" + std::to_string(id));
            getRequest->setMethod(Get);
            getRequest->setContentTypeCode(CT_APPLICATION_JSON);

            client->sendRequest(
                getRequest,
                [TEST_CTX, id](ReqResult getResult,
                               const HttpResponsePtr& getResponse)
                {
                    REQUIRE(getResult == ReqResult::Ok);
                    REQUIRE(getResponse != nullptr);
                    REQUIRE(getResponse->getStatusCode() == k200OK);

                    const auto fetchedUserJson = getResponse->getJsonObject();
                    REQUIRE(fetchedUserJson != nullptr);
                    CHECK((*fetchedUserJson)["name"] == "Alice");
                    CHECK((*fetchedUserJson)["age"] == 30);
                    CHECK((*fetchedUserJson)["id"] == id);
                }
            );
        }
    );
}

DROGON_TEST(userListTest)
{
    using namespace drogon;

    auto client = HttpClient::newHttpClient("http://127.0.0.1:5555");

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k200OK);

                auto json = resp->getJsonObject();

                REQUIRE(json != nullptr);
                CHECK((*json)["page"] == 1);
                CHECK((*json)["per_page"] == 10);
                REQUIRE((*json)["users"].isArray());
                CHECK((*json)["users"].size() <= 10);
            }
        );
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setQueryParameter("page", "2");
        req->setQueryParameter("per_page", "1");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k200OK);

                auto json = resp->getJsonObject();

                REQUIRE(json != nullptr);
                CHECK((*json)["page"] == 2);
                CHECK((*json)["per_page"] == 1);
                REQUIRE((*json)["users"].isArray());
                CHECK((*json)["users"].size() <= 1);
            }
        );
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setQueryParameter("page", "abc");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k400BadRequest);
            }
        );
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setQueryParameter("page", "9223372036854775807");
        req->setQueryParameter("per_page", "1");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k200OK);

                const auto json = resp->getJsonObject();
                REQUIRE(json != nullptr);
                REQUIRE((*json)["users"].isArray());
                CHECK((*json)["users"].empty());
            }
        );
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setQueryParameter("per_page", "101");
        req->setMethod(Get);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(
            req,
            [TEST_CTX](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == k400BadRequest);
            }
        );
    }

    struct PaginationCase
    {
        std::string key;
        std::string value;
        HttpStatusCode expectedStatus;
    };
    const PaginationCase cases[]{
        {"page", "0", k400BadRequest},
        {"page", "-1", k400BadRequest},
        {"per_page", "0", k400BadRequest},
        {"per_page", "abc", k400BadRequest},
        {"per_page", "100", k200OK}
    };
    for(const auto& testCase : cases)
    {
        auto req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setMethod(Get);
        req->setQueryParameter(testCase.key, testCase.value);
        client->sendRequest(
            req,
            [TEST_CTX, testCase](ReqResult result, const HttpResponsePtr& resp)
            {
                REQUIRE(result == ReqResult::Ok);
                REQUIRE(resp != nullptr);
                REQUIRE(resp->getStatusCode() == testCase.expectedStatus);
                const auto json = resp->getJsonObject();
                REQUIRE(json != nullptr);
                if(testCase.expectedStatus == k400BadRequest)
                {
                    REQUIRE((*json)["error"].isString());
                    CHECK(!(*json)["error"].asString().empty());
                    return;
                }
                CHECK((*json)["per_page"] == 100);
                const auto& users = (*json)["users"];
                REQUIRE(users.isArray());
                CHECK(users.size() <= 100);
                for(Json::ArrayIndex i = 1; i < users.size(); ++i)
                {
                    CHECK(users[i - 1]["id"].asInt64() < users[i]["id"].asInt64());
                }
            }
        );
    }
}

int main(int argc, char** argv)
{
    using namespace drogon;

    std::promise<void> loopStarted;
    auto loopStartedFuture = loopStarted.get_future();

    std::thread eventLoopThread([&loopStarted]()
    {
        app().getLoop()->queueInLoop([&loopStarted]()
        {
            loopStarted.set_value();
        });
        app().run();
    });

    loopStartedFuture.get();
    const int status = test::run(argc, argv);

    app().getLoop()->queueInLoop([]()
    {
        app().quit();
    });
    eventLoopThread.join();

    return status;
}
