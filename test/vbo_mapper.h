#ifndef _VBO_MAPPER_H_
#define _VBO_MAPPER_H_

#include <vector>
#include <imagedata.hpp>
#include <vbo.h>
#include <shader.h>

VBO* fromImageDataToVBO(nmath::ImageData<float> *imageData, ShaderVarLocation locations);
VBO* fromDataVectorToVBO(std::vector<float> bufferData, int stride, ShaderVarLocation locations);
VBO* fromDataToVBO(const float *bufferData, int verticeCount, int stride, ShaderVarLocation locations);

#endif