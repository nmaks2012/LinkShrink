#include "prometheus_metrics.hpp"

prometheus::Counter& linkshrink_urls_created_total;
prometheus::Counter& linkshrink_redirects_total;
prometheus::Histogram& linkshrink_db_latency;

prometheus::Registry& CreatePrometheusMetrics() {
  static prometheus::Registry registry;

  linkshrink_urls_created_total = prometheus::BuildCounter()
                                      .Name("linkshrink_urls_created_total")
                                      .Help("Total number of URLs created")
                                      .Register(registry);

  linkshrink_redirects_total = prometheus::BuildCounter()
                                   .Name("linkshrink_redirects_total")
                                   .Help("Total number of redirects")
                                   .Register(registry);

  linkshrink_db_latency = prometheus::BuildHistogram()
                              .Name("linkshrink_db_latency")
                              .Help("Histogram for database query latency")
                              .Register(registry)
                              .Buckets({0.1, 0.5, 1.0, 5.0, 10.0});

  return registry;
}
