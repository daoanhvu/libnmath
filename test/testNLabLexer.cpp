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
#include "logging.h"

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

void testLexicalAnalysis();
void testLexicalAnalysis_double_fail();

int main(int argc, char* argv[]) {
	testLexicalAnalysis_double_fail();
	testLexicalAnalysis();
	return 0;
}

void testLexicalAnalysis_double_fail() {
	std::string str = "1.5a";
	print_with_color(std::cout, "Test formula: " + str, FG_GREEN) << std::endl;
	nmath::NLabLexer lexer;
	vector<nmath::Token*> mTokens;
	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
	print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

	int errorCode = lexer.getErrorCode();
	int errorColumn = lexer.getErrorColumn();

	if(mTokens.size() != 0) {
		std::cout << "\033[31mTest failed!!! Token size = "<< mTokens.size() <<"\033[0m" << std::endl;
		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	}

	if(errorCode != ERROR_PARSING_NUMBER) {
		std::cout << "\033[31mTest failed!!! errorCode = "<< errorCode <<"\033[0m" << std::endl;
		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	}
	printError(errorColumn, errorCode);

	for(int i=0; i<mTokens.size(); i++) {
        delete mTokens[i];
    }
	mTokens.clear();
	std::cout << "===========================================================" << std::endl;

	str = "1..5";
	print_with_color(std::cout, "Test formula: " + str, FG_GREEN) << std::endl;
	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
	print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

	errorCode = lexer.getErrorCode();
	errorColumn = lexer.getErrorColumn();

	if(mTokens.size() != 0) {
		std::cout << "\033[31mTest failed!!! Token size = "<< mTokens.size() <<"\033[0m" << std::endl;
		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	}

	if(errorCode != ERROR_TOO_MANY_FLOATING_POINT) {
		std::cout << "\033[31mTest failed!!! errorCode = "<< errorCode <<"\033[0m" << std::endl;
		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	}
	printError(errorColumn, errorCode);

	for(int i=0; i<mTokens.size(); i++) {
        delete mTokens[i];
    }

	std::cout << "===========================================================" << std::endl;
}

// Test 1
void testLexicalAnalysis() {
	std::string str = "f(x)= x^2 D: x > 1.5";
	print_with_color(std::cout, "Test formula: " + str, FG_GREEN) << std::endl;
	nmath::NLabLexer lexer;
	vector<nmath::Token*> mTokens;
	int errorCode;
	int errorColumn;

	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
	print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

	if(mTokens.size() != 12) {
		std::cout << "\033[31mTest failed!!!\033[0m" << std::endl;
		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
	}

	for(int i=0; i<mTokens.size(); i++) {
        delete mTokens[i];
    }

    print_with_color(std::cout, "Test passed!!!!", FG_GREEN) << std::endl;
	std::cout << "===========================================================" << std::endl;
}