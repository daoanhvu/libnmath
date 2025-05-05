#include <gtest/gtest.h>
#include <string>
#include "command_handler.h"

// TEST(ParseIntegerTest, BasicAssertions) {
//   char text[] = {'1', 0x0};
//   int errorCode = 0;
//   int textLength = 1;
//   int value = nmath::parseInteger<int>(text, 0, textLength, &errorCode);
//   EXPECT_EQ(errorCode, 0);
//   EXPECT_EQ(value, 1);
// }

// TEST(ParseInvalidConeTest, BasicAssertions) {
//   std::string command = "cone(0, 0, 0, 1, 1, 1)";
//   ShaderVarLocation locations;
//   float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
//   std::vector<VboObject *> results;
//   CommandHandler commandHandler(locations);
//   commandHandler.handleCommand(command, color, results);
//   EXPECT_EQ(commandHandler.getErrorCode(), ERROR_INVALID_PARAMETER);
//   // Cleanup meshes
//   for(auto i=0; i< results.size(); i++) {
//     results[i]->release();
//     delete results[i];
//   }
// }

TEST(ParseValidConeTest, BasicAssertions) {
  std::string command = "cone(1, 10, 6, 10, 4)";
  ShaderVarLocation locations;
  float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  std::vector<VboObject *> results;
  CommandHandler commandHandler(locations);
  commandHandler.handleCommand(command, color, results);
  EXPECT_EQ(commandHandler.getErrorCode(), ERROR_INVALID_PARAMETER);
  // Cleanup meshes
  for(auto i=0; i< results.size(); i++) {
    results[i]->release();
    delete results[i];
  }
}

// TEST(ParseFunctionTest, BasicAssertions) {
//   std::string command = "f(x,y) = x^2 + y";
//   ShaderVarLocation locations;
//   float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
//   std::vector<VboObject *> results;
//   CommandHandler commandHandler(locations);
//   commandHandler.handleCommand(command, color, results);
//   EXPECT_EQ(commandHandler.getErrorCode(), 0);
//   // Cleanup meshes
//   for(auto i=0; i< results.size(); i++) {
//     results[i]->release();
//     delete results[i];
//   }
// }