// TestNMath.cpp : Defines the entry point for the console application.
//
#define GLM_FORCE_RADIANS

#include <SDKDDKVer.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <StringUtil.h>
#include <nmath.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <glm\gtx\matrix_cross_product.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <gm.hpp>
#include <camera.h>

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
			cout << "Too many floating point at "<< col << "\n";
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


void testDerivative() {
	DParam d;
	int error;
	double vars[] = {4, 1};
	double ret;
	char dstr[128];
	int l = 0;
	NFunction f;
	string str;

	cout << "Input function: ";
	cin >> str;	
	
	error = f.parse(str.c_str(), str.length());
	if(error != NMATH_NO_ERROR) {
		printError(f.getErrorColumn(), error);
		return;
	} 

	if( f.getVarCount() == 0 ) {
		cout << "This expression is not a function due to variables not determined.\n" ;
	}
	
	d.t = f.getPrefix(0);
	d.error = 0;
	d.returnValue = NULL;
	d.variables[0] = 'x';

	nmath::derivative(&d);
	//printNMAST(d.returnValue, 0, std::cout);
	l = 0;
	nmath::toString(d.returnValue, dstr, &l, 128);
	dstr[l] = '\0';
	cout << "f' = "<< dstr << "\n";
	
	clearTree(&(d.returnValue));

	f.release();
	clearPool();
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
				data = f.getSpace(interval, 0.2f, true);

				if (data != NULL) {
					for (i = 0; i<data->size; i++) {
						vcount = data->list[i]->dataSize / data->list[i]->dimension;
						cout << "Mesh " << i << ", row count: " << data->list[i]->rowCount << " number of vertex: " << vcount << "\n";
				
						for (j = 0; j<vcount; j++){
							cout << "x=" << data->list[i]->data[j * data->list[i]->dimension] << ", y = " << data->list[i]->data[j * data->list[i]->dimension + 1];

							if((data->list[i]->dimension)>=3) {
								cout << ", z = " << data->list[i]->data[j * data->list[i]->dimension + 2];
							}

							if((data->list[i]->dimension) >=4 ) {
								cout << ", nx = " << data->list[i]->data[j * data->list[i]->dimension + 3];
							}

							if((data->list[i]->dimension) >=5 ) {
								cout << ", ny = " << data->list[i]->data[j * data->list[i]->dimension + 4];
							}

							if((data->list[i]->dimension) >=6 ) {
								cout << ", nz = " << data->list[i]->data[j * data->list[i]->dimension + 5];
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

void printMat4(glm::mat4 *m) {
	int i, j;
	cout << "\n";
	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			cout << m->operator[](i)[j] << "\t";
		}
		cout << "\n";
	}
	cout << "\n";
}

void printMat4(gm::mat4 *m) {
	int i, j;
	cout << "\n";
	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			cout << m->operator[](i)[j] << "\t";
		}
		cout << "\n";
	}
	cout << "\n";
}


void testGM() {
	int i, j;
	glm::mat4 view1;
	glm::mat4 pers1;
	glm::mat4 pvm1;
	float fovy = D2R(35);
	float nearPlane = 0.5f;
	float farPlane = 9.5f;
	float aspect = 800.0f/600;

	fp::Camera camera;

	/* USE GLM */
	view1 = glm::lookAt(glm::vec3(0,0,-4), glm::vec3(0,0,0),glm::vec3(0,1,0));
	pers1 = glm::perspective(fovy, aspect, nearPlane, farPlane);
	cout << "GLM: \n";
	cout << "Aspect: " << aspect << "\n";
	cout <<"View: \n";
	printMat4(&view1);
	cout <<"Perspective: \n";
	printMat4(&pers1);
	cout <<"Multiply: \n";
	glm::mat4 mm1 = view1 * pers1;
	printMat4(&mm1);
	cout << "\n************************* \n";

	/* USE GM */
	cout << "GM: \n";
	cout <<"View: \n";
	camera.lookAt(0, 0, -4, 0, 0, 0, 0, 1, 0);
	camera.setViewport(0, 0, 800, 600);
	camera.setPerspective(fovy, nearPlane, farPlane);
	gm::mat4 view2 = camera.getView();
	gm::mat4 pers2 = camera.getPerspective();
	printMat4(&view2);
	printMat4(&pers2);
	cout <<"Multiply: \n";
	gm::mat4 mm2 = pers2 * view2;
	printMat4(&mm2);
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
			testDerivative();
			//jniJLexerGetSpace();
			//testGetSpaces();
			break;

		case 4:
			testCalculate();
			break;

		case 5:
			testGM();
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
