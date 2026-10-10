#include <sqlite3.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
    // unique_ptr 销毁时调用 sqlite3_close，负责关闭数据库连接。
    using DatabasePtr =
        std::unique_ptr<sqlite3, decltype(&sqlite3_close)>;

    std::string readSqlFile(const std::filesystem::path &path)
    {
        if (!std::filesystem::is_regular_file(path))
        {
            throw std::runtime_error(
                "SQL file is missing or is not a regular file: " +
                path.string());
        }

        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Failed to open SQL file: " + path.string());
        }

        std::ostringstream contents;
        contents << file.rdbuf();

        if (file.bad() || contents.fail())
        {
            throw std::runtime_error(
                "Failed to read SQL file: " + path.string());
        }

        std::string sql = contents.str();

        if (sql.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            throw std::runtime_error(
                "SQL file is empty: " + path.string());
        }

        return sql;
    }

    DatabasePtr openDatabase(const std::filesystem::path &path)
    {
        sqlite3 *rawDb = nullptr;
        const int code = sqlite3_open_v2(
            path.string().c_str(),
            &rawDb,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
            nullptr);

        // 打开失败也可能产生句柄，先接管它，再检查错误。
        DatabasePtr db(rawDb, sqlite3_close);

        if (code != SQLITE_OK)
        {
            const std::string error = db
                ? sqlite3_errmsg(db.get())
                : sqlite3_errstr(code);

            throw std::runtime_error(
                "Failed to open database '" + path.string() + "': " + error);
        }

        return db;
    }

    void executeSql(
        sqlite3 *db,
        const std::string &sql,
        const std::string &context)
    {
        // 一次执行完整脚本，包括 tasks 文件中的建表和建索引。
        const int code = sqlite3_exec(
            db,
            sql.c_str(),
            nullptr,
            nullptr,
            nullptr);

        if (code != SQLITE_OK)
        {
            throw std::runtime_error(
                "Failed to execute " + context + ": " + sqlite3_errmsg(db));
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr
            << "Usage: DatabaseInit <database-file> <migration-directory>\n";
        return 1;
    }

    try
    {
        const std::filesystem::path databasePath = argv[1];
        const std::filesystem::path migrationDir = argv[2];

        if (databasePath.empty() || migrationDir.empty())
        {
            throw std::invalid_argument(
                "Database file and migration directory must not be empty");
        }

        const auto usersFile = migrationDir / "001_create_users.sql";
        const auto tasksFile = migrationDir / "002_create_tasks.sql";

        // 先确认两份 SQL 都能读取，再创建数据库文件。
        const std::string usersSql = readSqlFile(usersFile);
        const std::string tasksSql = readSqlFile(tasksFile);

        auto db = openDatabase(databasePath);

        executeSql(db.get(), "PRAGMA foreign_keys = ON;", "foreign key setup");
        executeSql(db.get(), usersSql, usersFile.string());
        executeSql(db.get(), tasksSql, tasksFile.string());

        std::cout << "Database initialized: " << databasePath.string() << '\n';
        return 0;
    }
    catch (const std::exception &error)
    {
        // return 和异常展开会触发局部对象析构，关闭文件及数据库。
        std::cerr << "Database initialization failed: "
                  << error.what() << '\n';
        return 1;
    }
}
