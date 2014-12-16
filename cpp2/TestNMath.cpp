// TestNMath.cpp : Defines the entry point for the console application.
//

#include <SDKDDKVer.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <nmath.h>

using namespace nmath;

void printCriteria(Criteria *c) {
	int i, n;

	if (c == NULL) return;
	printf("\n");
	switch (c->getCClassType()){
		case SIMPLE:
			printf("(%lf,%lf)", ((SimpleCriteria*)c)->getLeftValue(), ((SimpleCriteria*)c)->getRightValue());
			break;
		case COMPOSITE:
			n = ((CompositeCriteria*)c)->size();
			for (i = 0; i < n; i++)
				printCriteria((*((CompositeCriteria*)c))[i]);
			break;
	}
}

void printMenu() {
	printf("\n0. Exit \n");
	printf("1. Test lexer \n");
	printf("2. Test Function \n");
	printf("3. Test getSpace \n");
	printf("4. Test Calculating \n");
	printf("-----------------------------------------------------------------------------------\n");
}

void testFunction() {
	NFunction f;
	ListFData *data;
	float interval[] = {-1, 2, 0, 1};
	int i, j, vcount, error, lineCount = 0;
	ifstream dataFile("D:\\data\\data.txt");
	string line;

	if(dataFile.is_open()) {
		while( getline(dataFile, line) ) {
			error = f.parse(line.c_str(), line.length());
			if (error != NMATH_NO_ERROR) {
				std::cout << "Function " << lineCount << " Parsing ERROR = " << error << "\n";
			}
			else {
				std::cout << f;
				data = f.getSpace(interval, 0.2f);

				if (data != NULL) {
					for (i = 0; i<data->size; i++) {
						vcount = data->list[i]->dataSize / data->list[i]->dimension;
						cout << "Mesh " << i << ", row count: " << data->list[i]->rowCount << " number of vertex: " << vcount << "\n";
				
						for (j = 0; j<vcount; j++){
							cout << "x=" << data->list[i]->data[j * data->list[i]->dimension] << ", y = " << data->list[i]->data[j * data->list[i]->dimension + 1];

							if((data->list[i]->dimension)>=3) {
								cout << ", z = " << data->list[i]->data[j * data->list[i]->dimension + 2];
							}
							cout << "\n";
						}

						free(data->list[i]->data);
						free(data->list[i]->rowInfo);
						free(data->list[i]);
					}
					free(data->list);
					free(data);
				}
				else {
					cout << "Function " << lineCount << " Get Space Failed \n";
				}
				cout << "\nFunction Parsing OK\n";
			}
			cout << "\n******************************************************\n";
			lineCount++;
		}

		dataFile.close();
	}
	f.release();
}

void testCalculate() {

}

void testCriteria(){
	NLabLexer lexer;
	NLabParser parser;
	NMAST *domain;
	Criteria *c, *o;
	char outStr[64];
	int start = 0;
	double value[] = { -1, 2, 0, 1 };
	int tokenCount = 50;
	Token tokens[50];
	int tokenInUse;
	ifstream dataFile("D:\\data\\criteria.txt");
	string line;

	if(dataFile.is_open()) {
		while( getline(dataFile, line) ) {
			tokenInUse = lexer.lexicalAnalysis(line.c_str(), line.length(), 0, tokens, tokenCount, 0);

			if(lexer.getErrorCode() != NMATH_NO_ERROR) {
				cout << "ERROR AT LEXICAL PHASE: \n";
				cout << "Error code = " << lexer.getErrorCode() << " at Column " << lexer.getErrorColumn() << "\n";
				continue;
			}

			domain = parser.parseDomain(tokens, tokenInUse, &start);

			if (parser.getErrorCode() == NMATH_NO_ERROR) {
				start = 0;
				printNMAST(domain, 0, cout);
				outStr[start] = '\0';
				puts(outStr);

				c = nmath::buildCriteria(domain);

				if (c->getCClassType() == COMPOSITE && ((CompositeCriteria*)c)->logicOperator() == OR)
					((CompositeCriteria*)c)->normalize("xy", 2);

				o = c->getInterval(value, "xy", 2);

				std::cout << ((Criteria&)*c) << "\n";
				std::cout << "\n Result: \n";
				std::cout << ((Criteria&)*o) << "\n";
				::clearTree(&domain);

				if (c != NULL)
					delete c;

				if (o != NULL)
					delete o;
			}
			else {
				cout << "ERROR AT PARSING PHASE: \n";
				cout << "Parsing error with code = " <<  parser.getErrorCode() << "\n";
			}
		}// end while

		dataFile.close();
	}
}

int _tmain(int argc, _TCHAR* argv[]) {
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
			testFunction();
			break;

		case 3:
			//jniJLexerGetSpace();
			//testGetSpaces();
			break;

		case 4:
			testCalculate();
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

	return 0;
}
