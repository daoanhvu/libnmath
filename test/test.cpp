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
#include "nfunction.hpp"
#include "criteria.hpp"
#include "SimpleCriteria.hpp"
#include "compositecriteria.hpp"
#include "logging.h"

using namespace nmath;

void printCriteria(Criteria<float> *c) {
	int n;

	if (c == NULL) return;
	printf("\n");
	switch (c->getCClassType()){
		case NMathCClassType::SIMPLE:
			printf("(%lf,%lf)", ((SimpleCriteria<float>*)c)->getLeftValue(), ((SimpleCriteria<float>*)c)->getRightValue());
			break;
		case NMathCClassType::COMPOSITE:
			n = ((CompositeCriteria<float>*)c)->size();
			for (int i = 0; i < n; i++) {
				printCriteria((*((CompositeCriteria<float>*)c))[i]);
			}
			break;
	}
}

void printMenu() {
	printf("\n0. Exit \n");
	printf("1. Test lexer \n");
	printf("2. Test Function \n");
	printf("3. Test getSpace \n");
	printf("4. Test Reduce \n");
	printf("5. Test Calculating \n");
	printf("-----------------------------------------------------------------------------------\n");
}

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
			break;
	}
}

void testCriteria() {
	std::string variable = "a";
	SimpleCriteria<float> *sc = new SimpleCriteria<float>(GTE_LTE, variable, (float)0, (float)0.5f, true, true);
	float value = 0.5f;
	if(sc->check(&value)) {
		std::cout << "\nPassed: " << value << " belong to the domain.\n";
	} else {
		std::cout << "\nFailed\n";
	}
	delete sc;
}

void testReduceOne(const std::string& line) {
	NFunction<float> f;
	NLabLexer lexer;
	NLabParser<float> parser;
	float interval[] = { -1, 2, 0, 1 };
	int i, j, vcount, error, lineCount = 0;
	int l = 0;
	char dstr[128];
	error = f.parse(line.c_str(), line.length(), &lexer, &parser);
	if (error != NMATH_NO_ERROR) {
		std::cout << "Expression " << lineCount << " Parsing ERROR = " << error << "\n";
	} else {
		std::cout << f;
		int resultCode = f.reduce();
		if (resultCode == NMATH_NO_ERROR) {
			toString<float>(f.getPrefix(0), dstr, &l, 128);
			dstr[l] = '\0';
			std::cout << "f = " << dstr << "\n";
		} else {
			std::cout << "Expression " << lineCount << " reduce Failed with code: "<< resultCode <<"\n";
		}
		std::cout << "Expression Parsing OK\n";
	}
	std::cout << "\n******************************************************\n";
}

void testCalculus(const std::string& line) {
	NFunction<float> f;
	NLabLexer lexer;
	NLabParser<float> parser;
	float interval[] = { -1, 2, 0, 1 };
	int i, j, vcount, error, lineCount = 0;
	int l = 0;
	char dstr[128];
	// error = f.parse(line.c_str(), line.length(), &lexer, &parser);
	error = f.parse(line, &lexer, &parser);
	if (error != NMATH_NO_ERROR) {
		std::cout << "Expression " << lineCount << " Parsing ERROR = " << error << "\n";
	} else {
		std::cout << f;
		int resultCode = f.reduce();
		if (resultCode == NMATH_NO_ERROR) {
			NMAST<float> *d = f.getDerivative("x");
			if(d != nullptr) {
				toString<float>(d, dstr, &l, 128);
				std::cout << "f'(x) = " << dstr << std::endl;
			}
			clearTree<float>(&d);
		} else {
			std::cout << "Expression " << lineCount << " reduce Failed with code: "<< resultCode <<"\n";
		}
		
	}
	std::cout << "\n******************************************************\n";
}

void testFunction() {
	std::string str = "f(x)= x^2 D: x > 1.5";
	print_with_color(std::cout, "Test Parser with formula: " + str, FG_GREEN) << std::endl;
	nmath::NLabLexer lexer;
	nmath::NLabParser<float> mParser;
	vector<nmath::Token*> mTokens;
	vector<nmath::NMAST<float>* > prefix;
	vector<nmath::Criteria<float>*> criteria;
	vector<nmath::NMAST<float>* > variables;
	int errorCode;
	int errorColumn;
	std::vector<nmath::NMAST<float>* > domain;

	lexer.lexicalAnalysis(str, false, 0, mTokens, nullptr);
	print_with_color(std::cout, "Number of token: ", FG_GREEN)  << mTokens.size() << std::endl;

	if(mTokens.size() != 12) {
		std::cout << "\033[31mTest failed!!!\033[0m" << std::endl;
		print_with_color(std::cout, "Test failed!!!!" + str, FG_RED) << std::endl;
		return;
	}

	mParser.parseFunctionExpression(mTokens, prefix,
                                             domain, variables, &errorCode, &errorColumn);
	std::cout << "Size of domain: " << domain.size() << std::endl;

	nmath::NMAST<float>* d1 = domain[0];

	nmath::Criteria<float> *c = nmath::buildCriteria(domain[0]);

	if(c->getCClassType() == nmath::SIMPLE) {
		nmath::SimpleCriteria<float> *sc = (nmath::SimpleCriteria<float>*)c;
		std::cout << " C is a \033[32mSIMPLE\033[0m criteria " << std::endl;
		std::cout << "type = " << sc->getType() << ", (L,R) = " << sc->getLeftValue() << ",  " << sc->getRightValue() << std::endl;
		if(sc->getType() != GT_LT) {
			print_with_color(std::cout, "Test failed!!!!", FG_RED) << std::endl;
		}
	} else {
		std::cout << " C is a COMPOSITE criteria " << std::endl;
	}

	for(int i=0; i<mTokens.size(); i++) {
        delete mTokens[i];
    }

	for(int i=0; i<prefix.size(); i++) {
        nmath::clearTree(&(prefix[i]));
    }

	for(int i=0; i<domain.size(); i++) {
        nmath::clearTree(&(domain[i]));
    }
    domain.clear();
    delete c;
    print_with_color(std::cout, "Test passed!!!!", FG_GREEN) << std::endl;
}

void testReduce() {
	std::string filename;
	NFunction<float> f;
	NLabLexer lexer;
	NLabParser<float> parser;
	float interval[] = { -1, 2, 0, 1 };
	int i, j, vcount, error, lineCount = 0;
	std::cout << "Enter data file: ";
	getline(std::cin, filename);
	std::ifstream dataFile(filename);
	std::string line;
	char dstr[128];
	int l = 0;

	if (dataFile.is_open()) {
		while (getline(dataFile, line)) {
			testReduceOne(line);
			lineCount++;
		}

		dataFile.close();
	}
	f.release();
}

int main(int argc, char* argv[]) {
	testFunction();
	// testReduceOne("f(x)=sin(x) + x");
	// testCriteria();
	testCalculus("f(x)=sin(x) + x");
	return 0;
}

int main1(int argc, char* argv[]) {
	int command;

	do {
		printMenu();
		printf("command = ");
		scanf("%d", &command);

		switch (command) {
		case 1:
			testCriteria();
			break;

		case 2:
			//testFunction0();
			// testFunction2(std::cout);
			break;

		case 3:
			// testDerivative();
			//jniJLexerGetSpace();
			//testGetSpaces();
			break;

		case 4:
			// testCalculate();
			break;

		case 5:
			testReduceOne(argv[1]);
			break;

		case 6:
			
			break;

		case 7:
			
			break;

		case 8:
			
			break;

		case 9:
			// testReduce();
			break;
		}

		//testReduce(f);
		//testDerivative(f);
		//	fflush(stdin);
		//	printf("Filename: ");
		//	fgets(str, 128, stdin);


		//testCalculate(f);
		//testReuseFunction(f);
		//testCriteria2(f);
	} while (command != 0);
	// releaseNMASTPool();

	return 0;
}
