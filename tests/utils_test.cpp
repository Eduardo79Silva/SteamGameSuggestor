#include "utils.h"
#include <gtest/gtest.h>

TEST(UtilsTest, UnixTimeToHumanReadable) {
  long int seconds = 1612137600;
  std::string expected = "1/2/2021 0:0:0";
  std::string result = unixTimeToHumanReadable(seconds);
  EXPECT_EQ(expected, result);
}
