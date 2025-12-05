#pragma once

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>
#include <userver/utils/periodic_task.hpp>
#include <vector>

#include "../models/click_info.hpp"

namespace linkshrink::storage {

class StatisticsAccumulator final : public userver::components::ComponentBase {
 public:

 static constexpr std::string_view kName = "statistics-accumulator";

  StatisticsAccumulator(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  ~StatisticsAccumulator() override;

  void OnAllComponentsLoaded() override;

  void PushBack(models::ClickInfo click_info);

 private:
  void WriteStatsToDB();

  userver::engine::Mutex mutex_;
  userver::storages::postgres::ClusterPtr pg_cluster_;
  userver::utils::PeriodicTask periodic_task_;
  std::vector<models::ClickInfo> metrics_data_;
};

}  // namespace linkshrink::storage