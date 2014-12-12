// TestNMath.cpp : Defines the entry point for the console application.
//

#include <SDKDDKVer.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
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
	int i, j, vcount;

	int error = f.parse("f(x)=x^2-2 D: x>0.5", 10);
	//int error = f.parse("f(x,y)=x^2-y D: x>0.3", 19);

	if (error != NMATH_NO_ERROR) {
		std::cout << "Function Parsing ERROR = " << error;
		f.release();
		return;
	}
	else {
		std::cout << f;
		//data = f.getSpace(interval, "xy", 2, 0.2f);
		data = f.getSpace(interval, "x", 1, 0.2f);

		if (data != NULL) {
			for (i = 0; i<data->size; i++){
				vcount = data->list[i]->dataSize / data->list[i]->dimension;
				cout << "Mesh " << i << ", row count: " << data->list[i]->rowCount << " number of vertex: " << vcount << "\n";
				
				for (j = 0; j<vcount; j++){
					cout << "x=" << data->list[i]->data[j * 2] << ", y = " << data->list[i]->data[j * 2 + 1] << "\n";
				}

				free(data->list[i]->data);
				free(data->list[i]->rowInfo);
				free(data->list[i]);
			}
			free(data->list);
			free(data);
		}
		else {
			cout << "Get Space Failed";
		}

		f.release();
		cout << "\nFunction Parsing OK\n";
	}
}

void testCalculate() {

}

void testCriteria(){
	NLabLexer lexer(10);
	NLabParser parser;
	NMASTList *domain;
	Criteria *c, *o;
	char outStr[64];
	int start = 0;
	double value[] = { 3, 12 };

	lexer.lexicalAnalysis("a > 10 and a <= 15", 18, 0);

	parser.parseDomain(lexer, &start);
	if (parser.getErrorCode() == NMATH_NO_ERROR) {
		domain = parser.domain();
		start = 0;
		printNMAST(domain->list[0], 0, cout);
		outStr[start] = '\0';
		puts(outStr);

		c = nmath::buildCriteria(domain->list[0]);

		o = c->getInterval(value, "a", 1);

		std::cout << ((Criteria&)*c) << "\n";
		std::cout << ((Criteria&)*o) << "\n";
		releaseNMATree(&domain);

		if (c != NULL)
			delete c;

		if (o != NULL)
			delete o;
	}
	else {
		printf("Parsing error with code = %d", parser.getErrorCode());
	}
}

int _tmain(int argc, _TCHAR* argv[]) {
	int command;

	do {
		printMenu();
		printf("command = ");
		scanf("%d", &command);

		switch (command) {
		case 0:
			testCriteria();
			break;

		case 1:
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
