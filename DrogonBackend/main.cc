#include <drogon/drogon.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::cerr << "Usage: DrogonBackend [config-file]\n";
        return 1;
    }

    const std::string configPath =
        argc == 2 ? argv[1] : "config.json";

    auto &app = drogon::app();
    try
    {
        app.loadConfigFile(configPath);

        app.registerBeginningAdvice([]
        {
            try
            {
                auto db = drogon::app().getDbClient("default");
                db->execSqlSync("PRAGMA foreign_keys = ON");

                const auto result = db->execSqlSync("PRAGMA foreign_keys");
                if (result.empty() || result[0][0].as<int>() != 1)
                {
                    throw std::runtime_error("Failed to enable SQLite foreign_keys");
                }

                LOG_INFO << "SQLite foreign_keys = 1";
            }
            catch (const std::exception &error)
            {
                LOG_ERROR << "Database initialization failed: " << error.what();
                std::exit(EXIT_FAILURE);
            }
        });
        app.run();
    }
    catch (const std::exception &error)
    {
        std::cerr << "Startup failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
