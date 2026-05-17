
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "common.hpp"
#include "nlablexer.h"
#include "nfunction.hpp"
#include "SimpleCriteria.hpp"
#include "imagedata.hpp"
#include "logging.h"

struct TestData {

	int testNumber;
	float epsilon;
	float values[4];
	int expectedNumOfSpace;
	int expectedRowCount;
	unsigned int expectedVertexCount;
};

void testFunction0();
void testGenerateIndices(const TestData &test);
void testCalculateDerivative();
void testCalculateSimpleDerivative();
void testCalculateProductSinDerivative();


int main(int argc, char* argv[]) {
	// testFunction0();

  testCalculateProductSinDerivative();

	// TestData test;
	// test.testNumber = 1;
	// test.epsilon = 0.5f;
	// test.values[0] = -1.0f;
	// test.values[1] = 1.0f;
	// test.values[2] = -1.0f;
	// test.values[3] = 1.0f;
	// test.expectedNumOfSpace = 1;
	// test.expectedRowCount = 5;
	// test.expectedVertexCount = 25;
	// testGenerateIndices(test);

	// test.testNumber = 2;
	// test.epsilon = 0.1f;
	// test.values[0] = -1.0f;
	// test.values[1] = 1.0f;
	// test.values[2] = -1.0f;
	// test.values[3] = 1.0f;
	// test.expectedNumOfSpace = 1;
	// test.expectedRowCount = 20;
	// testGenerateIndices(test);

	// test.testNumber = 2;
	// test.epsilon = 0.1f;
	// test.values[0] = -1.0f;
	// test.values[1] = 1.0f;
	// test.values[2] = -1.0f;
	// test.values[3] = 1.0f;
	// test.expectedNumOfSpace = 1;
	// test.expectedRowCount = 20;
	// testGenerateIndices(test);

	// test.testNumber = 3;
	// test.epsilon = 0.5f;
	// test.values[0] = -0.5f;
	// test.values[1] = 0.5f;
	// test.values[2] = -0.5f;
	// test.values[3] = 0.5f;
	// test.expectedNumOfSpace = 1;
	// test.expectedRowCount = 3;
	// test.expectedVertexCount = 9;
	// testGenerateIndices(test);
	return 0;
}

void testGenerateIndices(const TestData &test) {
	std::string inStr = "f(x,y)=x * sin(y)";
	nmath::NFunction<float> f;
	nmath::NLabLexer lexer;
	nmath::NLabParser<float> parser;
	int errorCode;
	int errorColumn;

	std::cout << "\033[32m" << "Test number " << test.testNumber << " parsing function: " << inStr << "\033[0m" << std::endl;

	errorCode = f.parse(inStr, &lexer, &parser);
	if(errorCode != NMATH_NO_ERROR) {
		print_with_color(std::cout, "Test failed! Cannot parse the expression " + inStr, FG_RED) << std::endl;
		return;
	}

	std::vector<nmath::ImageData<float>*> spaces = f.getSpace(test.values, test.epsilon, true, false);
	errorCode = f.getErrorCode();
	if(errorCode != NMATH_NO_ERROR) {
		print_with_color(std::cout, "Test failed! Cannot parse the expression " + inStr, FG_RED) << std::endl;
		for(auto i=0; i< spaces.size(); i++) {
			delete spaces[i];
		}
		return;
	}

	if(spaces.size() != test.expectedNumOfSpace) {
		print_with_color(std::cout, "Test failed!", FG_RED) << std::endl;
		print_with_color(std::cout, "\t Then number of space is ", FG_RED) << spaces.size()<< "\003[41m Expected: "<< test.expectedNumOfSpace <<"\033[0m" << std::endl;
	} else {
		nmath::ImageData<float>* imageData = spaces[0];
		std::cout << std::endl << "Row count = " << imageData->getRowCount() << std::endl;
		unsigned int rowCount = imageData->getRowCount();
		if(rowCount == test.expectedRowCount) {
			//OK
			for(auto i=0; i<rowCount; i++) {
				std::cout << "Row[" << i << "]: " <<  imageData->getRowAt(i) << std::endl;
			}
			unsigned int indexLen;
			unsigned short* indices = imageData->generateIndices(indexLen);
			std::cout << std::endl << "Indices( length = " << indexLen <<  ") " << std::endl;
			std::cout << "unsigned short indices[] = {";
			for(auto i=0; i<indexLen; i++) {
				std::cout << ", " << indices[i];
			}
			std::cout <<"};"<< std::endl;
			delete[] indices;

			if(imageData->getVertexCount() == test.expectedVertexCount) {
				auto vertexDataSize = imageData->vertexListSize();
				float *data = imageData->getData();
				std::cout << "float vertices[] = {";
				for(auto i=0; i<vertexDataSize; i++) {
					std::cout << ", " << data[i];
				}
				std::cout <<"};"<< std::endl;
				std::cout << "int vertexCount = " << imageData->getVertexCount() << ";" << std::endl;
				std::cout << "\033[32m" << "Test passed" << "\033[0m" << std::endl;
			} else {
				print_with_color(std::cout, "Test failed!", FG_RED) << std::endl;
				std::cout << "\033[31m\tVertexCount = "<< imageData->getVertexCount() << ", Expected: "<< test.expectedVertexCount <<"\033[0m" << std::endl;
			}
		} else {
			//Failed
			print_with_color(std::cout, "Test failed!", FG_RED) << std::endl;
			std::cout << "\033[31m\tRowCount = "<< rowCount << ", Expected: "<< test.expectedRowCount <<"\033[0m" << std::endl;
		}
	}

	for(auto i=0; i< spaces.size(); i++) {
		delete spaces[i];
	}

	std::cout << "=================================================" << std::endl;
}


void testFunction0() {
	nmath::NFunction<float> *f;
	nmath::NLabLexer lexer;
	nmath::NLabParser<float> parser;
	int tokenInUse;
	int errorCode;
	int errorColumn;
	std::ifstream dataFile("/Users/vdao/Documents/projects/demo/libnmath/test/criteria.txt");
	std::string line;
	float v[1] = {-2.0f};

	if(dataFile.is_open()) {
		while( getline(dataFile, line) ) {
			f = new nmath::NFunction<float>();
			std::cout << "Process " << line << std::endl;
			int error = f->parse(line, &lexer, &parser);
			if(error != NMATH_NO_ERROR) {
				std::cout << "Error with code: " << error << " at " << f->getErrorColumn() <<std::endl;
				print_with_color(std::cout, "Test failed! Cannot parse the expression " + line, FG_RED) << std::endl;
				continue;
			}

			float val = f->calc(v);
			errorCode = f->getErrorCode();
			if(errorCode != NMATH_NO_ERROR) {
				if(errorCode == ERROR_OUT_OF_DOMAIN) {
					print_with_color(std::cout, "Input value out of domain ", FG_GREEN) << std::endl;
				} else {
					print_with_color(std::cout, "Test failed!!!!", FG_RED) << std::endl;
				}
			} else {
				print_with_color(std::cout, "Value = ", FG_GREEN) << val << std::endl;
			}

			delete f;
		}// end while

		dataFile.close();
	}
}

void testCalculateSimpleDerivative() {
  std::string inStr = "x^2";
  // Postfix expression: x 2 ^
  std::vector<nmath::NMAST<float>*> postfix;

  /*
   * The AST tree should be:
   *    ^       
   *   / \     
   *  x  2  
   * 
   */

  // The first node x
  nmath::NMAST<float>* x = new nmath::NMAST<float>;
  x->type = VARIABLE;
  x->text = "x";
  x->parent = nullptr;
  x->left = nullptr;
  x->right = nullptr;
  postfix.push_back(x);

  nmath::NMAST<float>* number2 = new nmath::NMAST<float>;
  number2 = new nmath::NMAST<float>;
  number2->type = NUMBER;
  number2->text = "2";
  number2->value = 2.0f;
  number2->parent = nullptr;
  number2->left = nullptr;
  number2->right = nullptr;
  postfix.push_back(number2);

  nmath::NMAST<float>* power_1 = new nmath::NMAST<float>;
  power_1 = new nmath::NMAST<float>;
  power_1->type = POWER;
  power_1->text = "^";
  // base on getPriorityOfType()
  power_1->priority = 8;
  power_1->parent = nullptr;
  power_1->left = x;
  x->parent = power_1;
  power_1->right = number2;
  number2->parent = power_1;
  postfix.push_back(power_1);


  // Setup derivative parameters
  nmath::DParam<float> param;
  // Because we have a recursive expression tree and it's root is the last node in the postfix list
  // so for calculating the derivative of the function, we just need to get the last node in the postfix list
  param.t = postfix[postfix.size()-1];
  param.variables[0] = "x";
  param.varCount = 1;
  param.values[0] = 0.0f;
  param.error = 0;
  param.returnValue = nullptr;

  nmath::derivative<float>(&param);

  std::cout << "Derivative: " << param.returnValue->text << std::endl;
  std::cout << "Value: " << param.returnValue->value << std::endl;
  std::cout << "Priority: " << param.returnValue->priority << std::endl;
  std::cout << "Type: " << param.returnValue->type << std::endl;
  std::cout << "Sign: " << param.returnValue->sign << std::endl;
  std::cout << "Parent: " << param.returnValue->parent << std::endl;
  std::cout << "Left: " << param.returnValue->left << std::endl;
  std::cout << "Right: " << param.returnValue->right << std::endl;

  // Print the derivative tree
  std::cout << "Derivative tree: " << std::endl;
  printNMAST(param.returnValue, 0, std::cout);
  // nmath::reduce_t<float>(&param.returnValue);
  // std::cout << "Reduced derivative tree: " << std::endl;
  // printNMAST(param.returnValue, 0, std::cout);

  // TODO: Clean up the postfix list
  for(auto i=0; i<postfix.size(); i++) {
    delete postfix[i];
  }
  postfix.clear();

  // TODO: Clean up the derivative tree
  nmath::clearTree<float>(&param.returnValue);
}

void testCalculateProductSinDerivative() {
  // function: "x * sin(x)"
  // Postfix expression: x x sin *
  std::vector<nmath::NMAST<float>*> postfix;

  /*
   * The AST tree should be:
   *      *       
   *    /  \     
   *   x    sin
   *         /
   *        x
   */

  // The first node x
  nmath::NMAST<float>* x = new nmath::NMAST<float>;
  x->type = VARIABLE;
  x->text = "x";
  x->parent = nullptr;
  x->left = nullptr;
  x->right = nullptr;
  postfix.push_back(x);

  nmath::NMAST<float>* x2 = new nmath::NMAST<float>;
  x2->type = VARIABLE;
  x2->text = "x";
  x2->parent = nullptr;
  x2->left = nullptr;
  x2->right = nullptr;
  postfix.push_back(x2);

  nmath::NMAST<float>* nodeSin = new nmath::NMAST<float>;
  nodeSin = new nmath::NMAST<float>;
  nodeSin->type = SIN;
  nodeSin->text = "sin";
  // base on getPriorityOfType()
  nodeSin->priority = 0;
  nodeSin->parent = nullptr;
  nodeSin->left = x2;
  x2->parent = nodeSin;
  nodeSin->right = nullptr;
  postfix.push_back(nodeSin);

  nmath::NMAST<float>* mult = new nmath::NMAST<float>;
  mult = new nmath::NMAST<float>;
  mult->type = MULTIPLY;
  mult->text = "*";
  // base on getPriorityOfType()
  mult->priority = 5;
  mult->parent = nullptr;
  mult->left = x;
  x->parent = mult;
  mult->right = nodeSin;
  nodeSin->parent = mult;
  postfix.push_back(mult);


  // Setup derivative parameters
  nmath::DParam<float> param;
  // Because we have a recursive expression tree and it's root is the last node in the postfix list
  // so for calculating the derivative of the function, we just need to get the last node in the postfix list
  param.t = postfix[postfix.size()-1];
  param.variables[0] = "x";
  param.varCount = 1;
  param.values[0] = 0.0f;
  param.error = 0;
  param.returnValue = nullptr;

  nmath::derivative<float>(&param);

  std::cout << "Derivative: " << param.returnValue->text << std::endl;
  std::cout << "Value: " << param.returnValue->value << std::endl;
  std::cout << "Priority: " << param.returnValue->priority << std::endl;
  std::cout << "Type: " << param.returnValue->type << std::endl;
  std::cout << "Sign: " << param.returnValue->sign << std::endl;
  std::cout << "Parent: " << param.returnValue->parent << std::endl;
  std::cout << "Left: " << param.returnValue->left << std::endl;
  std::cout << "Right: " << param.returnValue->right << std::endl;

  // Print the derivative tree
  std::cout << "Derivative tree: " << std::endl;
  printNMAST(param.returnValue, 0, std::cout);
  // nmath::reduce_t<float>(&param.returnValue);
  // std::cout << "Reduced derivative tree: " << std::endl;
  // printNMAST(param.returnValue, 0, std::cout);

  // TODO: Clean up the postfix list
  for(auto i=0; i<postfix.size(); i++) {
    delete postfix[i];
  }
  postfix.clear();

  // TODO: Clean up the derivative tree
  nmath::clearTree<float>(&param.returnValue);
}

void testCalculateDerivative() {
  std::string inStr = "x * sin(x+2) + x/3";
  // Postfix expression: x x 2 + sin * x 3 / +
  std::vector<nmath::NMAST<float>*> postfix;

  /*
   * The AST tree should be:
   *         +
   *       /   \
   *     /      \
   *    *       %
   *   / \     /  \
   *  x  sin  x    3
   *     / 
   *    +
   *   / \
   *  x  2
   * 
   */

  // The first node x
  nmath::NMAST<float>* x1 = new nmath::NMAST<float>;
  x1->type = VARIABLE;
  x1->text = "x";
  x1->parent = nullptr;
  x1->left = nullptr;
  x1->right = nullptr;
  postfix.push_back(x1);

  nmath::NMAST<float>* x2 = new nmath::NMAST<float>;
  x2 = new nmath::NMAST<float>;
  x2->type = VARIABLE;
  x2->text = "x";
  x2->parent = nullptr;
  x2->left = nullptr;
  x2->right = nullptr;
  postfix.push_back(x2);

  nmath::NMAST<float>* number2_1 = new nmath::NMAST<float>;
  number2_1 = new nmath::NMAST<float>;
  number2_1->type = NUMBER;
  number2_1->text = "2";
  number2_1->value = 2.0f;
  number2_1->parent = nullptr;
  number2_1->left = nullptr;
  number2_1->right = nullptr;
  postfix.push_back(number2_1);

  nmath::NMAST<float>* plus_1 = new nmath::NMAST<float>;
  plus_1 = new nmath::NMAST<float>;
  plus_1->type = PLUS;
  plus_1->text = "+";
  // base on getPriorityOfType()
  plus_1->priority = 4;
  plus_1->parent = nullptr;
  plus_1->left = x2;
  x2->parent = plus_1;
  plus_1->right = number2_1;
  number2_1->parent = plus_1;
  postfix.push_back(plus_1);

  nmath::NMAST<float>* nodeSin = new nmath::NMAST<float>;
  nodeSin = new nmath::NMAST<float>;
  nodeSin->type = SIN;
  nodeSin->text = "sin";
  // base on getPriorityOfType()
  nodeSin->priority = 0;
  nodeSin->parent = nullptr;
  nodeSin->left = plus_1;
  plus_1->parent = nodeSin;
  nodeSin->right = nullptr;
  postfix.push_back(nodeSin);

  nmath::NMAST<float>* mult = new nmath::NMAST<float>;
  mult = new nmath::NMAST<float>;
  mult->type = MULTIPLY;
  mult->text = "*";
  // base on getPriorityOfType()
  mult->priority = 5;
  mult->parent = nullptr;
  mult->left = x1;
  x1->parent = mult;
  mult->right = nodeSin;
  nodeSin->parent = mult;
  postfix.push_back(mult);

  nmath::NMAST<float>* x3 = new nmath::NMAST<float>;
  x3 = new nmath::NMAST<float>;
  x3->type = VARIABLE;
  x3->text = "x";
  x3->parent = nullptr;
  x3->left = nullptr;
  x3->right = nullptr;
  postfix.push_back(x3);

  nmath::NMAST<float>* number3 = new nmath::NMAST<float>;
  number3 = new nmath::NMAST<float>;
  number3->type = NUMBER;
  number3->text = "3";
  number3->value = 3.0f;
  number3->parent = nullptr;
  number3->left = nullptr;
  number3->right = nullptr;
  postfix.push_back(number3);

  nmath::NMAST<float>* divide = new nmath::NMAST<float>;
  divide = new nmath::NMAST<float>;
  divide->type = DIVIDE;
  divide->text = "/";
  // base on getPriorityOfType()
  divide->priority = 5;
  divide->parent = nullptr;
  divide->left = x3;
  x3->parent = divide;
  divide->right = number3;
  number3->parent = divide;
  postfix.push_back(divide);

  nmath::NMAST<float>* plus_2 = new nmath::NMAST<float>;
  plus_2 = new nmath::NMAST<float>;
  plus_2->type = PLUS;
  plus_2->text = "+";
  // base on getPriorityOfType()
  plus_2->priority = 4;
  plus_2->parent = nullptr;
  plus_2->left = mult;
  mult->parent = plus_2;
  plus_2->right = divide;
  divide->parent = plus_2;
  postfix.push_back(plus_2);


  // Setup derivative parameters
  nmath::DParam<float> param;
  // Because we have a recursive expression tree and it's root is the last node in the postfix list
  // so for calculating the derivative of the function, we just need to get the last node in the postfix list
  param.t = postfix[postfix.size()-1];
  param.variables[0] = "x";
  param.varCount = 1;
  param.values[0] = 0.0f;
  param.error = 0;
  param.returnValue = nullptr;

  nmath::derivative<float>(&param);

  std::cout << "Derivative: " << param.returnValue->text << std::endl;
  std::cout << "Value: " << param.returnValue->value << std::endl;
  std::cout << "Priority: " << param.returnValue->priority << std::endl;
  std::cout << "Type: " << param.returnValue->type << std::endl;
  std::cout << "Sign: " << param.returnValue->sign << std::endl;
  std::cout << "Parent: " << param.returnValue->parent << std::endl;
  std::cout << "Left: " << param.returnValue->left << std::endl;
  std::cout << "Right: " << param.returnValue->right << std::endl;

  // Print the derivative tree
  std::cout << "Derivative tree: " << std::endl;
  printNMAST(param.returnValue, 0, std::cout);
  // nmath::reduce_t<float>(&param.returnValue);
  // std::cout << "Reduced derivative tree: " << std::endl;
  // printNMAST(param.returnValue, 0, std::cout);

  // TODO: Clean up the postfix list
  for(auto i=0; i<postfix.size(); i++) {
    delete postfix[i];
  }
  postfix.clear();

  // TODO: Clean up the derivative tree
  nmath::clearTree<float>(&param.returnValue);
}
