#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <vector>
#include "graphutil.h"

unsigned short* buildIndicesForGLTriangleStrip(const int* rowInfo, int rowInfoSize, unsigned int &len) {
	int rows = rowInfoSize;
	int cols = rowInfo[0];
	std::vector<unsigned short> indices;  
	for (auto y = 0; y<rows-1; y++) {
		if (y > 0) {
			// Degenerate begin: repeat first vertex
			indices.push_back((unsigned short)(y * cols));
		}
		
		for (int x = 0; x < cols; x++) {
			// One part of the strip
			indices.push_back((unsigned short) ((y * cols) + x));
			indices.push_back((unsigned short) (((y + 1) * cols) + x));
		}
		
		if (y < rows - 2) {
			// Degenerate end: repeat last vertex
			indices.push_back((unsigned short) (((y + 1) * cols) + (cols - 1)));
		}
	}

	unsigned int indSize = sizeof(unsigned short) * indices.size();
	unsigned short* results = new unsigned short[indices.size()];
	memcpy(results, (void*)(indices.data()), indSize);
	len = indices.size();

	return results;
}

unsigned short* buildIndicesForGLTriangleStrip(std::vector<int> rowInfo, unsigned int &len) {
	// Now build the index data
	int rows = rowInfo.size();	
	return buildIndicesForGLTriangleStrip(rowInfo.data(), rows, len);
}

/**
 * Just use this for quadratic vertex matrix
 * @param vcount
 * @param rows
 * @return
 */
int* buildIndicesForGLLINEs(int vcount, int *rows, int rowCount, int *size, int *loggedSize){
	int i, j, colCount=0, k;
	int *indices;
	int *tmp;
	
	*loggedSize = 100;
	indices = (int*)malloc(sizeof(int) * (*loggedSize));
	k = 0;
	*size = 0;
	for(i=0; i<rowCount; i++) {
		colCount = rows[i];
		for(j = 0; j<colCount; j++) {
			if(*size >= (*loggedSize)) {
				(*loggedSize) += 100;
				tmp = (int*)realloc(indices, sizeof(int) * (*loggedSize));
				indices = tmp;
			}
			indices[(*size)++] = k;
			if(j>0 && j<colCount-1) {
				if(*size >= (*loggedSize)) {
					(*loggedSize) += 100;
					tmp = (int*)realloc(indices, sizeof(int) * (*loggedSize));
					indices = tmp;
				}
				indices[(*size)++] = k;
			}
			k++;
		}
	}
	
	colCount = rows[0];
	for(j = 0; j<colCount; j++) {
		for(i=0; i<rowCount; i++) {
			if(*size >= (*loggedSize)) {
				(*loggedSize) += 100;
				tmp = (int*)realloc(indices, sizeof(int) * (*loggedSize));
				indices = tmp;
			}
			indices[(*size)++] = j + colCount * i;
			if(i> 0 && i<rowCount-1) {
				if(*size >= (*loggedSize)) {
					(*loggedSize) += 100;
					tmp = (int*)realloc(indices, sizeof(int) * (*loggedSize));
					indices = tmp;
				}
				indices[(*size)++] = j + colCount * i;
			}
		}
	}
	return indices;
}