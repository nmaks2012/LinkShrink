
#include "statistics_accumulator.hpp"
#include <chrono>
#include <optional>
#include <string>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/periodic_task.hpp>
#include <vector>

namespace linkshrink::storage {

StatisticsAccumulator::StatisticsAccumulator(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context),
      pg_cluster_(
          context
              .FindComponent<userver::components::Postgres>("postgres-main-db")
              .GetCluster()) {}

StatisticsAccumulator::~StatisticsAccumulator() { periodic_task_.Stop(); }
void StatisticsAccumulator::PushBack(models::ClickInfo click_info) {
  std::lock_guard<userver::engine::Mutex> lock(mutex_);
  metrics_data_.push_back(std::move(click_info));
}

void StatisticsAccumulator::OnAllComponentsLoaded() {
  using std::chrono::seconds;

  periodic_task_.Start("stats-writer",
                       userver::utils::PeriodicTask::Settings(seconds(5)),
                       [this] { this->WriteStatsToDB(); });
}

void StatisticsAccumulator::WriteStatsToDB() {
  std::vector<models::ClickInfo> db_metrics;
  {
    std::lock_guard<userver::engine::Mutex> lock(mutex_);
    if (metrics_data_.empty()) {
      return;
    }
    // Swap to local variable to release lock quickly
    db_metrics.swap(metrics_data_);
  }

  auto transaction =
      pg_cluster_->Begin(userver::storages::postgres::ClusterHostType::kMaster,
                         userver::storages::postgres::TransactionOptions{});

  try {
    for (const auto& click : db_metrics) {
      transaction.Execute(
          "INSERT INTO clicks (url_id, ts, user_agent, ip_address, referer, "
          "language, http_method, platform, is_mobile, country_code, trace_id) "
          "VALUES ($1, $2, $3, $4::inet, $5, $6, $7, $8, $9, $10, $11)",
          click.url_id, click.timestamp, click.user_agent, click.ip_address,
          click.referer, click.language, click.http_method, click.platform,
          click.is_mobile, click.country_code, click.trace_id);
    }
    transaction.Commit();
  } catch (const std::exception& e) {
    LOG_ERROR() << "Failed to write stats to DB: " << e.what();
  }
}

}  // namespace linkshrink::storage
