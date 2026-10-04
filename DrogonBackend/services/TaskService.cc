#include "TaskService.h"

#include "UserService.h"

#include <drogon/orm/Mapper.h>

#include <utility>

TaskService::TaskService(drogon::orm::DbClientPtr dbClient)
    : dbClient_(std::move(dbClient))
{
}

void TaskService::create(
    std::int64_t userId,
    std::string title,
    SuccessCallback onSuccess,
    ErrorCallback onError
) const
{
    UserService userService(dbClient_);

    // Both asynchronous stages need an error callback.
    auto onFindError = onError;

    userService.findById(
        userId,
        // Capture values instead of this: the service may be destroyed first.
        [dbClient = dbClient_,
         title = std::move(title),
         onSuccess = std::move(onSuccess),
         onError = std::move(onError)](UserService::User user) mutable
        {
            Task task;
            task.setUserId(user.getValueOfId());
            task.setTitle(std::move(title));
            task.setCompleted(0);

            drogon::orm::Mapper<Task> mapper(dbClient);
            mapper.insert(
                task,
                std::move(onSuccess),
                std::move(onError)
            );
        },
        std::move(onFindError)
    );
}
