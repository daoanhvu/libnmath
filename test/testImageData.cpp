
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


void testGenerateIndices() {
	int rowInfos[10] = {1, 3, 4, 2};
	nmath::ImageData<float> imageData(10, 3, 0, rowInfos, 4);
	imageData.generateIndices();
	std::cout << "Indices: ";
	imageData.printIndices(std::cout);
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
	int rowInfo[] = {2, 3};
	nmath::ImageData<float> imageData(5, 3, 0, rowInfo, 2);
	// const int rowCount = 2;
	// std::vector<short> actualResult;
	// const short expectedResult[] = {0, 2, 1, 3, 4};
	imageData.generateIndices();
	imageData.printIndices(std::cout);
	// printResult(expectedResult, 5, actualResult);

	// int rowInfo1[] = {3, 2, 2};
	// const int rowCount1 = 3;
	// std::vector<short> actualResult1;
	// const short expectedResult1[] = {0, 3, 1, 4, 2, 2, 3, 3, 5, 4, 6, 6};
	// generateIndices(rowInfo1, rowCount1, 7, actualResult1);
	// printResult(expectedResult1, 12, actualResult1);
	// // testGenerateIndices();

	// int rowInfo2[] = {3, 2};
	// const int rowCount2 = 2;
	// std::vector<short> actualResult2;
	// const short expectedResult2[] = {0, 3, 1, 4, 2, 2};
	// generateIndices(rowInfo2, rowCount2, 5, actualResult2);
	// printResult(expectedResult2, 6, actualResult2);

	// int rowInfo3[] = {3, 2, 3};
	// const int rowCount3 = 3;
	// std::vector<short> actualResult3;
	// const short expectedResult3[] = {0, 3, 1, 4, 2, 2, 3, 3, 5, 4, 6, 7};
	// generateIndices(rowInfo3, rowCount3, 8, actualResult3);
	// printResult(expectedResult3, 12, actualResult3);

	// int rowInfo4[] = {2, 4};
	// const int rowCount4 = 2;
	// std::vector<short> actualResult4;
	// const short expectedResult4[] = {0, 2, 1, 3, 4};
	// generateIndices(rowInfo4, rowCount4, 6, actualResult4);
	// printResult(expectedResult4, 5, actualResult4);

	// int rowInfo5[] = {2, 5};
	// const int rowCount5 = 2;
	// std::vector<short> actualResult5;
	// const short expectedResult5[] = {0, 2, 1, 3, 4};
	// generateIndices(rowInfo5, rowCount5, 7, actualResult5);
	// printResult(expectedResult5, 5, actualResult5);

	// int rowInfo6[] = {3, 3};
	// const int rowCount6 = 2;
	// std::vector<short> actualResult6;
	// const short expectedResult6[] = {0, 3, 1, 4, 2, 5, 5};
	// generateIndices(rowInfo6, rowCount6, 6, actualResult6);
	// printResult(expectedResult6, 7, actualResult6);

	// int rowInfo7[] = {5, 3};
	// const int rowCount7 = 2;
	// std::vector<short> actualResult7;
	// const short expectedResult7[] = {0, 5, 1, 6, 2, 7, 3, 4, 4};
	// generateIndices(rowInfo7, rowCount7, 8, actualResult7);
	// printResult(expectedResult7, 9, actualResult7);

	// int rowInfo8[] = {4, 2, 3};
	// const int rowCount8 = 3;
	// std::vector<short> actualResult8;
	// const short expectedResult8[] = {0, 4, 1, 5, 2, 3, 3, 4, 4, 6, 5, 7, 8, 8, 5, 5, 8, 8};
	// generateIndices(rowInfo8, rowCount8, 9, actualResult8);
	// printResult(expectedResult8, 18, actualResult8);

	return 0;
}