#include <userver/utest/utest.hpp>

#include <unordered_set>

#include "short_code.hpp"

UTEST(ShortCode, GeneratesCorrectLength) {
  constexpr size_t kLength = 8;
  const auto code = linkshrink::utils::GenerateShortCode(kLength);
  EXPECT_EQ(code.length(), kLength);
}

UTEST(ShortCode, IsAlphanumeric) {
  const auto code = linkshrink::utils::GenerateShortCode(100);
  for (char c : code) {
    EXPECT_TRUE(std::isalnum(static_cast<unsigned char>(c)))
        << "Character '" << c << "' is not alphanumeric";
  }
}

UTEST(ShortCode, IsRandom) {
  constexpr size_t kNumCodes = 1000;
  std::unordered_set<std::string> generated_codes;
  for (size_t i = 0; i < kNumCodes; ++i) {
    generated_codes.insert(linkshrink::utils::GenerateShortCode(8));
  }
  EXPECT_EQ(generated_codes.size(), kNumCodes);
}