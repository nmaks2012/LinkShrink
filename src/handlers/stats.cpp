#include "stats.hpp"

#include <optional>
#include <vector>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "../models/click_info.hpp"

namespace linkshrink::handlers {

namespace {
struct UrlClickRow {
  std::string original_url;
  std::optional<userver::storages::postgres::TimePointTz> ts;
  std::optional<std::string> user_agent;
  std::optional<std::string> ip_address;
  std::optional<std::string> referer;
  std::optional<std::string> language;
  std::optional<std::string> http_method;
  std::optional<std::string> platform;
  std::optional<bool> is_mobile;
  std::optional<std::string> country_code;
  std::optional<std::string> trace_id;
};
}  // namespace

StatsHandler::StatsHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      pg_cluster_(
          context.FindComponent<userver::components::Postgres>("postgres-main-db")
              .GetCluster()) {}

std::string StatsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/) const {
  const std::string path = std::string{request.GetRequestPath()};
  const size_t last_slash_pos = path.rfind('/');
  if (last_slash_pos == std::string::npos ||
      last_slash_pos + 1 == path.length()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    return "Invalid request path. Expected /stats/{short_code}";
  }
  const std::string short_code = path.substr(last_slash_pos + 1);

  const auto res = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      "SELECT u.original_url, c.ts, c.user_agent, c.ip_address::text, c.referer, c.language, "
      "c.http_method, c.platform, c.is_mobile, c.country_code, c.trace_id "
      "FROM urls u LEFT JOIN clicks c ON u.id = c.url_id "
      "WHERE u.short_code = $1 "
      "ORDER BY c.ts DESC",
      short_code);

  if (res.IsEmpty()) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    return "Not Found: URL for this short code does not exist.";
  }
  
  const auto rows = res.AsContainer<std::vector<UrlClickRow>>(userver::storages::postgres::kRowTag);

  userver::formats::json::ValueBuilder response_builder;
  response_builder["original_url"] = rows.front().original_url;

  userver::formats::json::ValueBuilder clicks_array(userver::formats::json::Type::kArray);
  for (const auto& row : rows) {
    if (row.ts.has_value()) {
      userver::formats::json::ValueBuilder click_builder;
      click_builder["timestamp"] = *row.ts;
      click_builder["user_agent"] = *row.user_agent;
      click_builder["ip_address"] = *row.ip_address;
      if (row.referer) click_builder["referer"] = *row.referer;
      if (row.language) click_builder["language"] = *row.language;
      click_builder["http_method"] = row.http_method.value_or("UNKNOWN");
      if (row.platform) click_builder["platform"] = *row.platform;
      click_builder["is_mobile"] = row.is_mobile.value_or(false);
      if (row.country_code) click_builder["country_code"] = *row.country_code;
      click_builder["trace_id"] = row.trace_id.value_or("");
      
      clicks_array.PushBack(click_builder.ExtractValue());
    }
  }
  response_builder["clicks"] = clicks_array.ExtractValue();

  request.GetHttpResponse().SetContentType("application/json");
  return userver::formats::json::ToString(response_builder.ExtractValue());
}

}  // namespace linkshrink::handlers