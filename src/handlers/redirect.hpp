#pragma once

#include <memory>
#include <string>
#include <userver/cache/expirable_lru_cache.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include "../storage/statistics_accumulator.hpp"

namespace linkshrink::handlers {

struct UrlInfo {
  long long id;
  std::string original_url;
};

class RedirectHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-redirect";

  RedirectHandler(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

 private:
  using Cache = userver::cache::ExpirableLruCache<std::string, UrlInfo>;
  UrlInfo UpdateCache(const std::string& key) const;
  userver::storages::postgres::ClusterPtr pg_cluster_;
  mutable Cache url_cache_;
  linkshrink::storage::StatisticsAccumulator& stats_accumulator_;
};

}  // namespace linkshrink::handlers