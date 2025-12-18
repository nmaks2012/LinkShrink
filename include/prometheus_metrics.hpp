#pragma once

#include <prometheus/counter.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>

extern prometheus::Counter* linkshrink_urls_created_total;
extern prometheus::Counter* linkshrink_redirects_total;
extern prometheus::Histogram* linkshrink_db_latency;

prometheus::Registry& CreatePrometheusMetrics();
