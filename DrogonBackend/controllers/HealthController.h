#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class HealthController : public drogon::HttpController<HealthController>
{
  public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(HealthController::health, "/health", drogon::Get);
  METHOD_LIST_END

  void health(
      const HttpRequestPtr&,
      std::function<void(const HttpResponsePtr&)>&& callback
  ) const;
};
