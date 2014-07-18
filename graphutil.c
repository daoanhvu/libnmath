#include <stdlib.h>
#include <stdio.h>
#include "graphutil.h"

/**
 * Just use this for quadratic vertex matrix
 * @param vcount
 * @param rows
 * @return
 */
short* buildIndicesForGLLINEs(int vcount, int *rows, int rowCount, int *size, int *loggedSize){
	int i, j, colCount=0, k;
	short *indices;
	short *tmp;
	
	*loggedSize = 100;
	indices = (short*)malloc(sizeof(short) * (*loggedSize));
	k = 0;
	*size = 0;
	for(i=0; i<rowCount; i++) {
		colCount = rows[i];
		for(j = 0; j<colCount; j++) {
			if(*size >= (*loggedSize)) {
				(*loggedSize) += 100;
				tmp = (short*)realloc(indices, sizeof(short) * (*loggedSize));
				indices = tmp;
			}
			indices[(*size)++] = k;
			if(j>0 && j<colCount-1) {
				if(*size >= (*loggedSize)) {
					(*loggedSize) += 100;
					tmp = (short*)realloc(indices, sizeof(short) * (*loggedSize));
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
				tmp = (short*)realloc(indices, sizeof(short) * (*loggedSize));
				indices = tmp;
			}
			indices[(*size)++] = j + colCount*i;
			if(i> 0 && i<rowCount-1) {
				if(*size >= (*loggedSize)) {
					(*loggedSize) += 100;
					tmp = (short*)realloc(indices, sizeof(short) * (*loggedSize));
					indices = tmp;
				}
				indices[(*size)++] = j + colCount*i;
			}
		}
	}
	return indices;
}