#include "handlers/prometheus_handler.hpp"
#include <prometheus/registry.h>
#include <prometheus/text_serializer.h>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response.hpp>

PrometheusMetricsHandler::PrometheusMetricsHandler(
    prometheus::Registry& registry)
    : registry_(registry) {}

void PrometheusMetricsHandler::HandleRequest(
    const userver::server::http::HttpRequest& request,
    userver::server::http::HttpResponse& response) {
  response.SetContentType("text/plain; version=0.0.4; charset=utf-8");
  response.SetBody(prometheus::WriteOpenMetricsTextFormat(registry_));
}
