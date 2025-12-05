#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/yaml_config/yaml_config.hpp>

#include "../utils/short_code.hpp"

namespace linkshrink::handlers {

class ShortenUrlHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-shorten-url";

  ShortenUrlHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

  static userver::yaml_config::Schema GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<
        userver::server::handlers::HttpHandlerBase>(R"(
        type: object
        description: Handler that creates a short URL.
        additionalProperties: false
        properties:
            base-url:
                type: string
                description: Base URL for the shortened links (e.g., http://localhost:8080)
        )");
  }

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
  std::string base_url_;
};

}  // namespace linkshrink::handlers
