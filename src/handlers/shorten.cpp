#include "shorten.hpp"

#include <string>
#include <chrono>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/result_set.hpp>
#include <userver/utils/rand.hpp>

#include "prometheus_metrics.hpp"

namespace linkshrink::handlers {

namespace {
constexpr size_t kShortCodeLength = 7;
}

ShortenUrlHandler::ShortenUrlHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      pg_cluster_(
          context
              .FindComponent<userver::components::Postgres>("postgres-main-db")
              .GetCluster()),
      base_url_(config["base-url"].As<std::string>()) {}

std::string ShortenUrlHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/) const {
  const auto request_json =
      userver::formats::json::FromString(request.RequestBody());
  const auto original_url = request_json["url"].As<std::string>();

  if (original_url.empty() || original_url.find("://") == std::string::npos) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return "Invalid 'url' field in request body.";
  }

  std::string successful_code;
  constexpr size_t kMaxAttempts = 10;

  auto trx = pg_cluster_->Begin(
      userver::storages::postgres::ClusterHostType::kMaster, {});

  for (size_t attempt = 0; attempt < kMaxAttempts; ++attempt) {
    const auto short_code =
        linkshrink::utils::GenerateShortCode(kShortCodeLength);
    try {
      auto start = std::chrono::steady_clock::now();
      auto res = trx.Execute(
          "INSERT INTO urls (original_url, short_code) VALUES ($1, $2) "
          "RETURNING short_code",
          original_url, short_code);
      auto end = std::chrono::steady_clock::now();
      if (linkshrink_db_latency) {
        linkshrink_db_latency->Observe(
            std::chrono::duration<double>(end - start).count());
      }

      successful_code = res.AsSingleRow<std::string>();

      if (linkshrink_urls_created_total) {
        linkshrink_urls_created_total->Increment();
      }
      break;

    } catch (const userver::storages::postgres::UniqueViolation& e) {
      continue;
    }
  }

  if (successful_code.empty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kInternalServerError);
    return "Could not generate a unique URL. Please try again later.";
  }

  trx.Commit();

  userver::formats::json::ValueBuilder response_builder;
  response_builder["short_url"] = base_url_ + "/" + successful_code;

  request.GetHttpResponse().SetContentType("application/json");
  return userver::formats::json::ToString(response_builder.ExtractValue());
}

}  // namespace linkshrink::handlers
