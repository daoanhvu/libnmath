
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


int main(int argc, char* argv[]) {
	// testFunction0();

	TestData test;
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

	test.testNumber = 3;
	test.epsilon = 1.0f;
	test.values[0] = -0.5f;
	test.values[1] = 0.5f;
	test.values[2] = -0.5f;
	test.values[3] = 0.5f;
	test.expectedNumOfSpace = 1;
	test.expectedRowCount = 2;
	test.expectedVertexCount = 4;
	testGenerateIndices(test);
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

	std::vector<nmath::ImageData<float>*> spaces = f.getSpace(test.values, test.epsilon, true);
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
			imageData->generateIndices();
			std::cout << std::endl << "Indices( length = " << imageData->indicesSize() <<  ") " << std::endl;
			unsigned short *indices = imageData->getIndices();
			std::cout << "unsigned short indices[] = {";
			for(auto i=0; i<imageData->indicesSize(); i++) {
				std::cout << ", " << indices[i];
			}
			std::cout <<"};"<< std::endl;

			if(imageData->getVertexCount() == test.expectedVertexCount) {
				auto vertexDataSize = imageData->vertexListSize();
				float *data = imageData->getData();
				std::cout << "float vertices[] = {";
				for(auto i=0; i<vertexDataSize; i++) {
					std::cout << ", " << data[i];
				}
				std::cout <<"};"<< std::endl;
			} else {

			}

			std::cout << "\033[32m" << "Test passed" << "\033[0m" << std::endl;
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
