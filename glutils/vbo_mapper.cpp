#include "vbo_mapper.h"
#include "graphutil.h"
#include <iostream>

VBO* fromImageDataToVBO(nmath::ImageData<float> *imageData, ShaderVarLocation locations) {
    // It needs 3 floats for position, 3 floats for normal and 3 floats for color
    int floatStride = 9;
    unsigned int verticeCount = imageData->getVertexCount();
    std::cout << "Going to generate buffer data, number of vertices: " << verticeCount << std::endl;
    float *verticesBufferData = new float[verticeCount * floatStride];
    float red = 0.3f;
    float green = 0.5f;
    float blue = 0.4f;
    unsigned int bufferLen = imageData->copyDataWithColorTo(red, green, blue, verticesBufferData);
    std::cout << "Done generate buffer data, buffer len: " << bufferLen << std::endl;
    unsigned int indexCount;
    auto rowCount = imageData->getRowCount();
    unsigned short *indices = buildIndicesForGLTriangleStrip(imageData->getRowInfo(), rowCount, indexCount);
    std::cout << "Generate indices DONE, indexCount = " << indexCount << std::endl;
    std::cout << "Row count " << imageData->getRowCount() << std::endl;
    VBO* vbo = new VBO(verticesBufferData, verticeCount, indices, indexCount, locations, true);

    //Now we can release memory for vertices data
    delete[] verticesBufferData;
    delete[] indices;

    return vbo;
}

VBO* fromDataVectorToVBO(std::vector<float> bufferData, int stride, ShaderVarLocation locations) {
    return nullptr;
}

VBO* fromDataToVBO(const float *bufferData, int verticeCount, 
int stride, ShaderVarLocation locations, bool useNormal) {
    return new VBO(bufferData, verticeCount, nullptr, 0, locations, useNormal);
}