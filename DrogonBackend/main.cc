#include <drogon/drogon.h>

int main()
{
    auto& app = drogon::app();
    app.loadConfigFile("config.json");

    app.registerBeginningAdvice([]{
        try{
            auto db = drogon::app().getDbClient("default");
            db->execSqlSync("PRAGMA foreign_keys = ON");

            const auto result = db->execSqlSync("PRAGMA foreign_keys");
            if(result.empty() || result[0][0].as<int>() != 1)
            {
                throw std::runtime_error("Failed to enable SQLite foreign_keys");
            }

            LOG_INFO << "SQLite foreign_keys = 1";
        }
        catch(const std::exception& error)
        {
            LOG_ERROR << "Database initialization failed: " << error.what();
            std::exit(EXIT_FAILURE);
        }
    });
    app.addListener("0.0.0.0", 5555);
    app.run();
    return 0;
}
