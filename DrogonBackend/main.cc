#include <drogon/drogon.h>
int main() {
    //Set HTTP listener address and port
    drogon::app().addListener("0.0.0.0", 5555);

    drogon::app().registerHandler(
        "/health",
        [](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        )
        {
            Json::Value body;
            body["status"] = "OK";

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(body);

            callback(response);
        },
        {drogon::Get}
    );
    //Load config file
    //drogon::app().loadConfigFile("../config.json");
    //drogon::app().loadConfigFile("../config.yaml");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
