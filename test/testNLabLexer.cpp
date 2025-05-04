// TestNMath.cpp : Defines the entry point for the console application.
//
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include "nlablexer.h"
#include "gtest/gtest.h"

using namespace nmath;

void printError(int col, int code) {
	switch(code){
		case ERROR_DIV_BY_ZERO:
			break;

		case ERROR_LOG:
			break;

		case ERROR_OPERAND_MISSING:
			break;

		case ERROR_PARSE:
			break;

		case ERROR_TOO_MANY_FLOATING_POINT:
			std::cout << "Too many floating point at "<< col << "\n";
			break;

		case ERROR_PARENTHESE_MISSING:
			printf("Missing parenthese at %d\n", col);
			break;
			
		case ERROR_TOO_MANY_PARENTHESE:
			printf("Too many parenthese at %d\n", col);
			break;

		case ERROR_OUT_OF_DOMAIN:
			break;

		case ERROR_SYNTAX:
			break;

		case ERROR_NOT_AN_EXPRESSION:
			printf("Bad expression found at %d\n", col);
			break;

		case ERROR_NOT_A_FUNCTION:
			printf("Bad function notation found at %d\n", col);
			break;

		case ERROR_MISSING_FUNCTION_NOTATION:
			printf("This expression is not a function due to variables not determined.\n");
			break;

		case ERROR_BAD_TOKEN:
			printf("A bad token found at %d\n", col);
			break;

		case ERROR_LEXER:
			break;

		case ERROR_PARSING_NUMBER:
			std::cout << "Number format exception at "<< col << std::endl;
			break;
	}
}

TEST(FloatingPointErrorTest, BasicAssertions) {
	std::string str = "1.5a";
	nmath::NLabLexer lexer;
	vector<nmath::Token*> mTokens;
	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);

  ASSERT_EQ(mTokens.size(), 0);

  int errorCode = lexer.getErrorCode();
	int errorColumn = lexer.getErrorColumn();

	if(errorCode != ERROR_PARSING_NUMBER) {
		std::cout << "\033[31mTest failed!!! errorCode = "<< errorCode <<"\033[0m" << std::endl;
	}
	ASSERT_EQ(errorCode, ERROR_PARSING_NUMBER);
  ASSERT_EQ(errorColumn, 3);

	for(int i=0; i<mTokens.size(); i++) {
    delete mTokens[i];
  }
	mTokens.clear();
}

TEST(ParentheseFloatingPointTest, BasicAssertions) {
	std::string str = "(1.5,";
	nmath::NLabLexer lexer;
	vector<nmath::Token*> mTokens;
	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
  ASSERT_EQ(mTokens.size(), 3);

  int errorCode = lexer.getErrorCode();
	int errorColumn = lexer.getErrorColumn();
	ASSERT_EQ(errorCode, 0);
  ASSERT_EQ(errorColumn, -1);

	for(int i=0; i<mTokens.size(); i++) {
    delete mTokens[i];
  }
	mTokens.clear();
}

	// std::cout << "===========================================================" << std::endl;

	// str = "1..5";
	// print_with_color(std::cout, "Test formula: " + str, FG_GREEN) << std::endl;
	// lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
	// print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

	// errorCode = lexer.getErrorCode();
	// errorColumn = lexer.getErrorColumn();

	// if(mTokens.size() != 0) {
	// 	std::cout << "\033[31mTest failed!!! Token size = "<< mTokens.size() <<"\033[0m" << std::endl;
	// 	print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	// }

	// if(errorCode != ERROR_TOO_MANY_FLOATING_POINT) {
	// 	std::cout << "\033[31mTest failed!!! errorCode = "<< errorCode <<"\033[0m" << std::endl;
	// 	print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	// }
	// printError(errorColumn, errorCode);

	// for(int i=0; i<mTokens.size(); i++) {
  //       delete mTokens[i];
  //   }

	// std::cout << "===========================================================" << std::endl;

	// str = "12.54+a";
	// print_with_color(std::cout, "Test formula: " + str, FG_GREEN) << std::endl;
	// lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
	// print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

	// errorCode = lexer.getErrorCode();
	// errorColumn = lexer.getErrorColumn();

	// if(mTokens.size() != 3) {
	// 	std::cout << "\033[31mTest failed!!! Token size = "<< mTokens.size() <<"\033[0m" << std::endl;
	// 	print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	// }

	// if(errorCode != NMATH_NO_ERROR) {
	// 	std::cout << "\033[31mTest failed!!! errorCode = "<< errorCode <<"\033[0m" << std::endl;
	// 	print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	// 	printError(errorColumn, errorCode);
	// }

	// for(int i=0; i<mTokens.size(); i++) {
  //       delete mTokens[i];
  //   }

	// std::cout << "===========================================================" << std::endl;

/**
 * Test for the cone function
 * cone(height, radius, capRadius, stacks, capStacks)
 */
TEST(ConeFunctionTest, BasicAssertions) {
	std::string str = "cone(1.5, 12, 6, 10, 6)";
	nmath::NLabLexer lexer;
	vector<nmath::Token*> tokens;
  int lastMeanIdx = -1;
	unsigned int tokenCount = lexer.lexicalAnalysis(str, false, 0, tokens, &lastMeanIdx);

  // For debugging
  // std::cout << "Token count: " << tokenCount << std::endl;
  // std::cout << "Last Mean Index: " << lastMeanIdx << std::endl;

  // for (size_t i = 0; i < tokenCount; ++i) {
  //   std::cout << "Token " << i << ": " << tokens[i]->text << std::endl;
  //   std::cout << "Type: " << tokens[i]->type << std::endl;
  //   std::cout << "Column: " << tokens[i]->column << std::endl;
  //   std::cout << "Text Length: " << (int)tokens[i]->textLength << std::endl;
  //   std::cout << "Priority: " << tokens[i]->priority << std::endl;
  //   std::cout << "------------------------" << std::endl;
  // }

  ASSERT_EQ(tokens.size(), 12);

  ASSERT_EQ(tokens[0]->type, NAME);

  int errorCode = lexer.getErrorCode();
	int errorColumn = lexer.getErrorColumn();

  ASSERT_EQ(errorCode, NMATH_NO_ERROR);
  ASSERT_EQ(errorColumn, -1);

	for(int i=0; i<tokens.size(); i++) {
    delete tokens[i];
  }
	tokens.clear();
}

// Test 1
// void testLexicalAnalysis() {
// 	std::string str = "f(x)= x^2 D: x > 1.5";
// 	print_with_color(std::cout, "Test formula: " + str, FG_GREEN) << std::endl;
// 	nmath::NLabLexer lexer;
// 	vector<nmath::Token*> mTokens;
// 	int errorCode;
// 	int errorColumn;

// 	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
// 	errorCode = lexer.getErrorCode();
// 	errorColumn = lexer.getErrorColumn();
// 	print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

// 	if(mTokens.size() != 12) {
// 		std::cout << "\033[31mTest failed!!!\033[0m" << std::endl;
// 		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
// 	}

// 	if( (errorCode != NMATH_NO_ERROR) || (errorColumn != -1)) {
// 		print_with_color(std::cout, "Test failed!!!! " + str, FG_RED) << std::endl;
// 		std::cout << "\033[31m\terrorCode = "<< errorCode << ", Expected: "<< NMATH_NO_ERROR <<"\033[0m" << std::endl;
// 		std::cout << "\033[31m\terrorColumn = "<< errorColumn << ", Expected: " << -1 <<"\033[0m" << std::endl;
// 		printError(errorColumn, errorCode);
// 	}

// 	for(int i=0; i<mTokens.size(); i++) {
//         delete mTokens[i];
//     }

//     print_with_color(std::cout, "Test passed!!!!", FG_GREEN) << std::endl;
// 	std::cout << "===========================================================" << std::endl;
// }