#include <gtest/gtest.h>
#include <string>
#include "function_utils.h"

// Demonstrate some basic assertions.
TEST(ParseCommandTest, BasicAssertions) {
  std::string command = "cone(0, 0, 0, 1, 1, 1)";
  int result = parseCommand(command);
  EXPECT_EQ(result, 0); // Assuming parseCommand returns 0 on success
  // // Expect two strings not to be equal.
  // EXPECT_STRNE("hello", "world");
  // // Expect equality.
  // EXPECT_EQ(7 * 6, 42);
}