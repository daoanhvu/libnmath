// TestNMath.cpp : Defines the entry point for the console application.
//
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include <common.hpp>
#include "nlablexer.h"

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

int testCone(int argc, char* argv[]) {
	std::string str = "cone(0, 0, 0, 1, 1, 1)";
	nmath::NLabLexer lexer;
	vector<nmath::Token*> mTokens;
	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);

  int errorCode = lexer.getErrorCode();
	int errorColumn = lexer.getErrorColumn();

	for(int i=0; i<mTokens.size(); i++) {
    delete mTokens[i];
  }
	mTokens.clear();
  return 0;
}

int main(int argc, char* argv[]) {
	char text[] = {'1', 0x0};
  int errorCode = 0;
  int textLength = 1;
  int value = nmath::parseInteger<int>(text, 0, textLength, &errorCode);
  std::cout << "Parsed integer: " << value << ", Error code: " << errorCode << std::endl;
  return 0;
}
