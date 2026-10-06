#include "TaskService.h"

#include "UserService.h"

#include <drogon/orm/Criteria.h>
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

void TaskService::listByUserId(
    std::int64_t userId,
    ListCallback onSuccess,
    ErrorCallback onError
) const
{
    UserService userService(dbClient_);

    auto onFindError = onError;

    userService.findById(
        userId,
        [dbClient = dbClient_,
         onSuccess = std::move(onSuccess),
         onError = std::move(onError)](UserService::User user) mutable
        {
            drogon::orm::Mapper<Task> mapper(dbClient);

            mapper.orderBy(Task::Cols::_id)
                .findBy(
                    drogon::orm::Criteria(
                        Task::Cols::_user_id, user.getValueOfId()),
                    std::move(onSuccess),
                    std::move(onError)
                );
        },
        std::move(onFindError)
    );
}

void TaskService::updateCompleted(
    std::int64_t userId,
    std::int64_t taskId,
    bool completed,
    AffectedRowsCallback onSuccess,
    ErrorCallback onError
) const
{
    drogon::orm::Mapper<Task> mapper(dbClient_);

    // Match the task and its owner in the same UPDATE statement.
    const auto criteria =
        drogon::orm::Criteria(Task::Cols::_id, taskId) &&
        drogon::orm::Criteria(Task::Cols::_user_id, userId);

    mapper.updateBy(
        {Task::Cols::_completed},
        std::move(onSuccess),
        std::move(onError),
        criteria,
        completed ? std::int64_t{1} : std::int64_t{0}
    );
}
