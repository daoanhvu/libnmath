// TestNMath.cpp : Defines the entry point for the console application.
//
#define GLM_FORCE_RADIANS

#include <SDKDDKVer.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <StringUtil.h>
#include <nmath.h>
#include <glm\glm.hpp>
#include <glm\ext.hpp>
#include <glm\gtx\matrix_cross_product.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <gm.hpp>
#include <camera.h>
#include <nmath_pool.h>

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
	NLabLexer lexer;
	NLabParser parser;
	string str;

	cout << "Input function: ";
	cin >> str;	
	
	error = f.parse(str.c_str(), str.length(), &lexer, &parser);
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

void testFunction0() {
	NFunction f;
	NLabLexer lexer;
	NLabParser parser;
	nmath::DParam reduceParam;
	int error = f.parse("f(x,y)=sin(x)*y", 15, &lexer, &parser);

	nmath::NMAST *dx = f.getDerivativeByVariable(0, 0);
	reduceParam.t = dx;
	reduce_t((void*)&reduceParam);
	dx = reduceParam.t;

	nmath::NMAST *dy = f.getDerivativeByVariable(0, 1);
	reduceParam.t = dy;
	reduce_t((void*)&reduceParam);
	dy = reduceParam.t;

	printNMAST(dx, 0, std::cout);
	printNMAST(dy, 0, std::cout);

	clearTree(&dx);
	clearTree(&dy);
}

//write space to binary file
void testFunction1() {
	NFunction f;
	NLabLexer lexer;
	NLabParser parser;

	ListFData *data;
	float interval[] = { -1.5f, 1.5f, -1.5f, 1.5f };
	float v;
	int i, j, vcount, tmp, spaceCount, error, lineCount = 0;
	ifstream dataFile("D:\\data\\data.txt");
	ofstream outFile("D:\\data\\function.dat", std::ofstream::binary);
	string line;

	if (dataFile.is_open()) {
		while (getline(dataFile, line)) {
			error = f.parse(line.c_str(), line.length(), &lexer, &parser);
			if (error != NMATH_NO_ERROR) {
				std::cout << "Function " << lineCount << " Parsing ERROR = " << error << "\n";
			}
			else {
				//std::cout << f;
				data = f.getSpace(interval, 0.5f, true);

				if (data != NULL) {
					spaceCount = data->size;
					outFile.write((char*)&spaceCount, sizeof(int));
					for (i = 0; i<data->size; i++) {
						
						vcount = data->list[i]->dataSize / data->list[i]->dimension;
						tmp = data->list[i]->rowCount;
						outFile.write((char*)&vcount, sizeof(int));
						outFile.write((char*)&tmp, sizeof(int));
						tmp = data->list[i]->dimension;
						outFile.write((char*)&tmp, sizeof(int));

						for (j = 0; j<data->list[i]->dataSize; j++){
							v = data->list[i]->data[j];
							outFile.write((char*)&v, sizeof(float));
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
		outFile.flush();
		outFile.close();
		dataFile.close();
	}
	f.release();
}

/**
* Need to be tested with OpenGL
* @param vertices
* @param rows each element in this array contains the number of vertex on the corresponding row
* @return
*/
void buildIndicesForTriangleStrip(int vcount, int* rows, int rowCount) {
	int i, j, num_of_vertices, nextCol, count, diff, k;

	std::vector<int> indices;

	count = 0;
	for (i = 0; i<rowCount - 1; i++){
		num_of_vertices = rows[i];
		nextCol = rows[i + 1];

		for (j = 0; j<num_of_vertices; j++){
			indices.push_back(count);
			if (i>0 && j == 0){ //first element of rows that not the first row
				indices.push_back(count);
			}

			if (j == num_of_vertices - 1){
				if (i < rowCount - 2){
					if ((count + num_of_vertices) < vcount && j<nextCol){
						//neu co 1 phan tu ngay ben duoi
						indices.push_back(count + num_of_vertices);
						indices.push_back(count);
					}
					indices.push_back(count);
				}
				else	if ((count + num_of_vertices) < vcount && j<nextCol){
					k = count + num_of_vertices;
					while (k < vcount){
						indices.push_back(k);
						k++;
						if (k < vcount)
							indices.push_back(k);
						k++;
						if (k<vcount)
							indices.push_back(count);
					}
				}
			}
			else{
				//neu ngay ben duoc co mot vertex nua
				if ((count + num_of_vertices) < vcount && j<nextCol)
					indices.push_back(count + num_of_vertices);
				else{ //neu khong thi add vertex cuoi cung cua dong duoi
					diff = j - nextCol + 1;
					indices.push_back(count + num_of_vertices - diff);
				}
			}
			count++;
		}
	}
	int outSize = indices.size();
	std::cout << "Indices length: " << outSize << "\n";
	for (i = 0; i<outSize; i++) {
		std::cout << indices[i] << "  ";
	}
	std::cout << "\n";

}

void testFunction2(std::ostream &out) {
	NFunction f;
	NLabLexer lexer;
	NLabParser parser;

	ListFData *data;
	float interval[] = {-0.5f, 1.0f, -0.5f, 1.0f};
	int i, j, vcount, error, lineCount = 0;
	ifstream dataFile("D:\\data\\data.txt");
	string line;

	if(dataFile.is_open()) {
		while( getline(dataFile, line) ) {
			error = f.parse(line.c_str(), line.length(), &lexer, &parser);
			if (error != NMATH_NO_ERROR) {
				std::cout << "Function " << lineCount << " Parsing ERROR = " << error << "\n";
			}
			else {
				//std::cout << f;
				data = f.getSpace(interval, 0.5f, true);

				if (data != NULL) {
					for (i = 0; i<data->size; i++) {
						vcount = data->list[i]->dataSize / data->list[i]->dimension;
						cout << "Mesh " << i << ", row count: " << data->list[i]->rowCount << " number of vertex: " << vcount << "\n";
						buildIndicesForTriangleStrip(vcount, data->list[i]->rowInfo, data->list[i]->rowCount);
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
								cout << ", nz = " << std::setw(10) << std::setprecision(6) << data->list[i]->data[j * data->list[i]->dimension + 5];
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

void testReduce() {
	NFunction f;
	NLabLexer lexer;
	NLabParser parser;

	float interval[] = { -1, 2, 0, 1 };
	int i, j, vcount, error, lineCount = 0;
	ifstream dataFile("D:\\data\\expression.txt");
	string line;
	char dstr[128];
	int l = 0;

	if (dataFile.is_open()) {
		while (getline(dataFile, line)) {
			error = f.parse(line.c_str(), line.length(), &lexer, &parser);
			if (error != NMATH_NO_ERROR) {
				std::cout << "Expression " << lineCount << " Parsing ERROR = " << error << "\n";
			}
			else {
				std::cout << f;
				if (f.reduce() == NMATH_NO_ERROR) {
					nmath::toString(f.getPrefix(0), dstr, &l, 128);
					dstr[l] = '\0';
					cout << "f' = " << dstr << "\n";
				}
				else {
					cout << "Expression " << lineCount << " reduce Failed \n";
				}
				cout << "\Expression Parsing OK\n";
			}
			cout << "\n******************************************************\n";
			lineCount++;
		}

		dataFile.close();
	}
	f.release();
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

				std::cout << line << "\n";
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
	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			cout << m->operator[](i)[j] << "\t";
		}
		cout << "\n";
	}
	cout << "\n";
}

float angle2DVector(float x1, float y1, float x2, float y2) {
	float d = (x1 * x2) + (y1 * y2);
	float cs = d / (sqrt(x1*x1 + y1*y1) * sqrt(x2*x2 + y2*y2));
	return acos(cs);
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

void testProject() {
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
	glm::vec3 obj(1.2f, 0.75, 0.03);
	glm::vec3 a = glm::project(obj, view1, pers1, glm::vec4(0, 0, 800, 600));
	cout << "GLM: \n";
	cout << "Aspect: " << aspect << "\n";
	cout <<"View: \n";
	printMat4(&view1);
	cout <<"Perspective: \n";
	printMat4(&pers1);
	cout << "Out: (" << a[0] << ", " << a[1] << ", " << a[2] << ")\n";
	cout << "\n************************* \n";

	/* USE GM */
	cout << "GM: \n";
	cout <<"View: \n";
	float a1[3];
	camera.lookAt(0, 0, -4, 0, 0, 0, 0, 1, 0);
	camera.setViewport(0, 0, 800, 600);
	camera.setPerspective(fovy, nearPlane, farPlane);
	gm::mat4 view2 = camera.getView();
	gm::mat4 pers2 = camera.getPerspective();
	camera.project(a1, 1.2f, 0.75, 0.03);
	printMat4(&view2);
	printMat4(&pers2);
	
	cout << "Out: (" << a1[0] << ", " << a1[1] << ", " << a1[2] << ")";
}

void testMultiply() {
	int i, j;
	glm::mat4 view1;
	fp::Camera camera;

	/* USE GLM */
	view1 = glm::lookAt(glm::vec3(0,0,-4), glm::vec3(0,0,0),glm::vec3(0,1,0));
	glm::vec4 obj(1.2f, 0.75, 0.03, 2);
	glm::vec4 a = view1 * obj;
	cout << "GLM: \n";
	
	cout <<"View: \n";
	printMat4(&view1);
	cout << "(" << obj[0] << ", " << obj[1] << ", " << obj[2] << ", " << obj[3] << ")\n";
	cout <<"Result: (" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ")\n";
	cout << "\n************************* \n";

	/* USE GM */
	cout << "GM: \n";
	cout <<"View: \n";
	camera.lookAt(0, 0, -4, 0, 0, 0, 0, 1, 0);
	camera.setViewport(0, 0, 800, 600);
	gm::mat4 view2 = camera.getView();
	printMat4(&view2);
	cout << "(" << obj[0] << ", " << obj[1] << ", " << obj[2] << ", " << obj[3] << ")\n";
	gm::vec4 tmp(obj[0], obj[1], obj[2], obj[3]);
	gm::vec4 a1 = view2 * tmp;
	cout << "Out: (" << a1[0] << ", " << a1[1] << ", " << a1[2] << ", " << a1[3] << ")";
}

void testCamera() {
	int i, j;
	fp::Camera camera;
	float a1[3];

	/* USE GM */
	cout << "GM: \n";
	cout <<"View: \n";
	camera.lookAt(0, 0, -6.5f, 0, 0, 0, 0, 1, 0);
	camera.setViewport(0, 0, 600, 886);
	camera.setPerspective(D2R(35), 0.1f, 9.0f);
	gm::mat4 view2 = camera.getView();
	gm::mat4 pers2 = camera.getPerspective();

	printMat4(&view2);
	printMat4(&pers2);

	//camera.project(a1, 1.0f, 0.0f, 0.0f);
	camera.project(a1, 0.0f, 0.0f, 0.0f);
	cout << "Out: (" << a1[0] << ", " << a1[1] << ", " << a1[2] << ")";
}


int _tmain(int argc, _TCHAR* argv[]) {
	int command;

	initNMASTPool();
	do {
		printMenu();
		printf("command = ");
		scanf("%d", &command);

		switch (command) {
		case 1:
			testCriteria();
			break;

		case 2:
			testFunction0();
			//testFunction2(std::cout);
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

		case 6:
			testProject();
			break;

		case 7:
			testMultiply();
			break;

		case 8:
				testCamera();
			break;

		case 9:
			testReduce();
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
	releaseNMASTPool();

	return 0;
}
