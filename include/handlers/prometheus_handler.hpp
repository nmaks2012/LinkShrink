#pragma once

#include <prometheus/registry.h>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response.hpp>

class PrometheusMetricsHandler : public userver::server::http::HttpRequestHandler {
public:
    explicit PrometheusMetricsHandler(prometheus::Registry& registry);

    void HandleRequest(const userver::server::http::HttpRequest& request,
                       userver::server::http::HttpResponse& response) override;

private:
    prometheus::Registry& registry_;
};
