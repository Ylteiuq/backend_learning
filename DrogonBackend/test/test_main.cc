#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>

DROGON_TEST(BasicTest)
{
    using namespace drogon;

    
    auto client = HttpClient::newHttpClient("http://127.0.0.1:5555");
    {    
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setMethod(Post);
        req->setContentTypeCode(CT_APPLICATION_JSON);
        req->setBody("not-json");

        client->sendRequest(req, [TEST_CTX](ReqResult result, const HttpResponsePtr& resp) {
            REQUIRE(result == ReqResult::Ok);
            REQUIRE(resp != nullptr);
            REQUIRE(resp->getStatusCode() == k400BadRequest);
        });
    }

    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setBody(R"({"name":"Alice"})");
        req->setMethod(Post);
        req->setContentTypeCode(CT_APPLICATION_JSON);

        client->sendRequest(req, [TEST_CTX](ReqResult result, const HttpResponsePtr& resp){
            REQUIRE(result == ReqResult::Ok);
            REQUIRE(resp != nullptr);
            REQUIRE(resp->getStatusCode() == k400BadRequest);
        });
    }
}

DROGON_TEST(normalTest)
{
    using namespace drogon;

    auto client = HttpClient::newHttpClient("http://127.0.0.1:5555");
    {
        HttpRequestPtr req = HttpRequest::newHttpRequest();
        req->setPath("/users");
        req->setMethod(Post);
        req->setContentTypeCode(CT_APPLICATION_JSON);
        req->setBody(R"({"name":"Alice","age":30})");

        client->sendRequest(req, [TEST_CTX](ReqResult result, const HttpResponsePtr& resp) {
            REQUIRE(result == ReqResult::Ok);
            REQUIRE(resp != nullptr);
            CHECK(resp->getStatusCode() == k201Created);
            auto json = resp->getJsonObject();
            CHECK((*json)["name"] == "Alice");
            CHECK((*json)["age"] == 30);
        });
    }
}

int main(int argc, char** argv) 
{
    using namespace drogon;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
