
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "common.hpp"
#include "imagedata.hpp"
#include "nlablexer.h"
#include "nfunction.hpp"
#include "SimpleCriteria.hpp"
#include "logging.h"

struct GeneratedIndicesTestCase {
	public:
	int id;
	int rowInfo[32];
	int rowCount;
	short expectedResult[256];
	int expectedResultSize;

	GeneratedIndicesTestCase() {
		rowCount = 0;
	}

	GeneratedIndicesTestCase(int _id, const int *rows, int _rowCount, const short *expecteds, int expectedRlen) {
		this->id = _id;
		memcpy(this->rowInfo, rows, sizeof(int) * _rowCount);
		this->rowCount = _rowCount;
		memcpy(this->expectedResult, expecteds, sizeof(short) * expectedRlen);
		this->expectedResultSize = expectedRlen;
	}
} ;


bool runTestCase(const GeneratedIndicesTestCase *test) {
	int vertexCount = 0;
	bool result = true;
	for(auto i=0; i<test->rowCount; i++) {
		vertexCount += test->rowInfo[i];
	}
	nmath::ImageData<float> imageData(vertexCount, 3, 0, test->rowInfo, test->rowCount);
	imageData.generateIndices();
	std::vector<short> actualResult = imageData.getIndices();
	if(actualResult.size() != test->expectedResultSize) {
		std::cout << "\033[41m" << "Test failed! actual size: " << actualResult.size() << "\033[0m" << std::endl;
		return false;
	} else {
		for(auto i=0; i<test->expectedResultSize; i++) {
			if(actualResult[i] != test->expectedResult[i]) {
				std::cout << "\033[41m" << "Test failed! actual value: " << actualResult[i] << ". Expected value: " << test->expectedResult[i] << "\033[0m" << std::endl;
				return false;
			}
		}
		result = true;
	}
	std::cout << "\033[32m" << "Test " << test->id << " passed" << "\033[0m" << std::endl;
	std::cout << "======================================" << std::endl;
	return result;
}

void runTestes() {
	int rowInfo1[] = {3, 2};
	const short expectedResult1[] = {0, 3, 1, 4, 2};
	GeneratedIndicesTestCase test1(1, rowInfo1, 2, expectedResult1, 5);
	runTestCase(&test1);

	int rowInfo2[] = {2, 3};
	const short expectedResult2[] = {0, 2, 1, 3, 4};
	GeneratedIndicesTestCase test2(2, rowInfo2, 2, expectedResult2, 5);
	runTestCase(&test2);

	int rowInfo3[] = {2, 2, 2};
	const short expectedResult3[] = {0, 2, 1, 3, 3, 2, 2, 4, 3, 5};
	GeneratedIndicesTestCase test3(3, rowInfo3, 3, expectedResult3, 10);
	runTestCase(&test3);
}

void printResult(const short *expected, int len, std::vector<short>& indices) {
	std::cout << std::endl << "Expected( length = " << len <<  "): ";
	for(auto i=0; i<len; i++) {
		std::cout << " " << expected[i];
	}
	std::cout << std::endl << "Indices( length = " << indices.size() <<  "):  ";
	for(auto i=0; i<indices.size(); i++) {
		std::cout << " " << indices[i];
	}
	std::cout << std::endl << "===============================================" << std::endl;
}


int main(int argc, char* argv[]) {

	if(argc < 3) {
		runTestes();
		return 0;
	}
	int error;
	int *rowInfo = new int[argc -1];
	int numOfVertex = 0;

	for(auto i=1; i<argc; i++) {
		auto len = strlen(argv[i]);
		auto val = nmath::parseInteger<int>(argv[i], 0, len, &error);
		rowInfo[i-1] = val;
		numOfVertex += val;
	}
	int numRow = argc - 1;

	nmath::ImageData<float> imageData(numOfVertex, 3, 0, rowInfo, numRow);
	imageData.generateIndices();
	imageData.printIndices(std::cout);

	delete[] rowInfo;

	// int rowInfo8[] = {4, 2, 3};
	// const int rowCount8 = 3;
	// std::vector<short> actualResult8;
	// const short expectedResult8[] = {0, 4, 1, 5, 2, 3, 3, 4, 4, 6, 5, 7, 8, 8, 5, 5, 8, 8};
	// generateIndices(rowInfo8, rowCount8, 9, actualResult8);
	// printResult(expectedResult8, 18, actualResult8);

	return 0;
}