#include "short_code.hpp"

#include <userver/utils/rand.hpp>

namespace linkshrink::utils {

namespace {
constexpr std::string_view kAlphabet =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
}  // namespace

std::string GenerateShortCode(size_t length) {
  std::string result(length, 'a');
  for (auto& c : result) {
    const auto val = userver::utils::RandRange(kAlphabet.size());
    c = kAlphabet[val];
  }

  return result;
}

}  // namespace linkshrink::utils
