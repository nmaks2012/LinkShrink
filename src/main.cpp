#include <userver/clients/dns/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_static.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/components/fs_cache.hpp>
#include <userver/clients/http/component.hpp>

#include "handlers/redirect.hpp"
#include "handlers/shorten.hpp"
#include "handlers/stats.hpp" 
#include "storage/statistics_accumulator.hpp"

int main(int argc, char* argv[]) {

  auto component_list = userver::components::MinimalServerComponentList()
                           .Append<userver::server::handlers::Ping>()
                           .Append<userver::server::handlers::HttpHandlerStatic>("handler-static")
                           .Append<userver::server::handlers::TestsControl>()
                           .Append<userver::components::Postgres>("postgres-main-db")
                           .Append<userver::components::FsCache>("fs-cache-for-static")
                           .Append<userver::components::TestsuiteSupport>()
                           .Append<userver::components::HttpClient>() 
                           .Append<userver::components::HttpClientCore>("http-client-core")                       
                           .Append<userver::clients::dns::Component>()
                           .Append<linkshrink::handlers::ShortenUrlHandler>()
                           .Append<linkshrink::handlers::RedirectHandler>()
                           .Append<linkshrink::handlers::StatsHandler>()
                           .Append<linkshrink::storage::StatisticsAccumulator>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}