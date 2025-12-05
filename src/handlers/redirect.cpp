#include "redirect.hpp"

#include <functional>
#include <string>
#include <userver/cache/expirable_lru_cache.hpp>
#include <userver/cache/lru_cache_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/result_set.hpp>
#include <userver/tracing/span.hpp>

#include "../models/click_info.hpp"

namespace linkshrink::handlers {

RedirectHandler::RedirectHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      pg_cluster_(
          context
              .FindComponent<userver::components::Postgres>("postgres-main-db")
              .GetCluster()),
      url_cache_(16, 1000),
      stats_accumulator_(
          context.FindComponent<linkshrink::storage::StatisticsAccumulator>()) {
}

std::string RedirectHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/) const {
  const std::string short_code =
      std::string{request.GetRequestPath().substr(1)};

  if (short_code.empty()) {
    auto& response = request.GetHttpResponse();
    response.SetStatus(userver::server::http::HttpStatus::kFound);
    response.SetHeader(std::string_view{"Location"}, "/static/index.html");
    return {};
  }

  auto update_func = [this](const std::string& key) {
    return UpdateCache(key);
  };

  UrlInfo url_info;

  try {
    url_info = url_cache_.Get(short_code, update_func);
  } catch (const std::exception& e) {
    LOG_ERROR() << e.what();
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    return "Not Found: URL for this short code does not exist.";
  }

  std::string referer_str = request.GetHeader("Referer");
  std::string lang_str = request.GetHeader("Accept-Language");

  std::string platform_str = request.GetHeader("Sec-CH-UA-Platform");
  std::string mobile_str = request.GetHeader("Sec-CH-UA-Mobile");
  std::string country_str = request.GetHeader("CF-IPCountry");

  models::ClickInfo click{
      .url_id = url_info.id,
      .timestamp =
          userver::storages::postgres::TimePointTz{
              std::chrono::system_clock::now()},
      .user_agent = std::string{request.GetHeader("User-Agent")},
      .ip_address = request.GetRemoteAddress().PrimaryAddressString(),
      .referer =
          referer_str.empty() ? std::nullopt : std::optional{referer_str},
      .language = lang_str.empty() ? std::nullopt : std::optional{lang_str},
      .http_method = request.GetMethodStr(),
      .platform =
          platform_str.empty() ? std::nullopt : std::optional{platform_str},
      .is_mobile = (mobile_str == "?1"),
      .country_code =
          country_str.empty() ? std::nullopt : std::optional{country_str},
      .trace_id =
          std::string(userver::tracing::Span::CurrentSpan().GetTraceId())};

  /// accumulate
  stats_accumulator_.PushBack(click);

  auto& response = request.GetHttpResponse();
  response.SetStatus(userver::server::http::HttpStatus::kFound);
  response.SetHeader(std::string_view{"Location"}, url_info.original_url);

  return {};
}

UrlInfo RedirectHandler::UpdateCache(const std::string& key) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "SELECT id, original_url FROM urls WHERE short_code = $1", key);
  return result.AsSingleRow<UrlInfo>(userver::storages::postgres::kRowTag);
}

}  // namespace linkshrink::handlers