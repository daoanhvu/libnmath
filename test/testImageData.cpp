
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
	nmath::ImageData<float> imageData(10, 0, rowInfos, 4);
	imageData.generateIndices();
	std::cout << "Indices: ";
	imageData.printIndices(std::cout);
}

void generateIndices(const int* rowInfo, int rowCount) {
	std::vector<short> indices;
	short count = 0;
	for(int i=0; i<rowCount-1; i++) {
		auto vertexCountLine = rowInfo[i];
        auto nexRowCount = rowInfo[i + 1];
		for(int j=0; j<vertexCountLine; j++) {
			if(j == vertexCountLine - 1) {
				indices.push_back(count);
				// add the vertex right below
				indices.push_back(count + vertexCountLine);
				int e = count + vertexCountLine + 1;
				int e1 = count + nexRowCount + 1;
				for(auto k=e; k<e1; k++) {
					indices.push_back(k);
					indices.push_back(k);
					indices.push_back(count);
					indices.push_back(count);
					indices.push_back(k);
				}
			} else {
				indices.push_back(count);
				// add the vertex right below
				indices.push_back(count + vertexCountLine);
			}
			count++;
		}
	}

	std::cout << std::endl << "Indices( length = " << indices.size() <<  "): ";
	for(auto i=0; i<indices.size(); i++) {
		std::cout << " " << indices[i];
	}
	std::cout << std::endl;
}


int main(int argc, char* argv[]) {
	int rowInfo[10] = {3, 2};
	const int rowCount = 2;
	generateIndices(rowInfo, rowCount);
	// testGenerateIndices();
	return 0;
}