#ifndef __GRAPHUTIL_H_
#define __GRAPHUTIL_H_

#include <vector>

unsigned short* buildIndicesForGLTriangleStrip(std::vector<int> rowInfo, unsigned int &len);
unsigned short* buildIndicesForGLTriangleStrip(const int* rowInfo, int rowInfoSize, 
    unsigned int &len);
int* buildIndicesForGLLINEs(int vcount, int *rows, int rowCount, int *size, 
    int *loggedSize);

#endif