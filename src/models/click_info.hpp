#pragma once

#include <chrono>
#include <string>
#include <optional>

#include <userver/storages/postgres/io/chrono.hpp>

namespace linkshrink::models {

struct ClickInfo {
  long long url_id;
  userver::storages::postgres::TimePointTz timestamp;
  
  std::string user_agent;
  std::string ip_address;
  std::optional<std::string> referer;
  std::optional<std::string> language;
  
  std::string http_method;
  std::optional<std::string> platform;
  bool is_mobile;
  std::optional<std::string> country_code;
  std::string trace_id;

  bool operator==(const ClickInfo& other) const {
      return timestamp.GetUnderlying() == other.timestamp.GetUnderlying() &&
             user_agent == other.user_agent &&
             ip_address == other.ip_address &&
             referer == other.referer &&
             language == other.language &&
             http_method == other.http_method &&
             platform == other.platform &&
             is_mobile == other.is_mobile &&
             country_code == other.country_code &&
             trace_id == other.trace_id;
  }
};

}  // namespace linkshrink::models