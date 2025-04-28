#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "function_utils.h"
#include "cone.hpp"

int generateMeshAndIndices(std::string inputFunction, const float *values, float epsilon, 
  ShaderVarLocation locations, std::vector<VboObject*> &results) {
    nmath::NFunction<float> f;
    nmath::NLabLexer lexer;
    nmath::NLabParser<float> parser;
    int errorCode;
    int errorColumn;
  
    errorCode = f.parse(inputFunction, &lexer, &parser);
    if (errorCode != NMATH_NO_ERROR) {
      std::cerr << "Error parsing function: " << errorCode << std::endl;
      return -1;
    }
  
    std::vector<nmath::ImageData<float>*> spaces = f.getSpace(values, epsilon, true, false);
    errorCode = f.getErrorCode();
    if(errorCode != NMATH_NO_ERROR) {
      std::cerr << "Test failed! Cannot parse the expression " << inputFunction << " with error code " << errorCode << std::endl;
      for(auto i=0; i< spaces.size(); i++) {
        delete spaces[i];
      }
      return -1;
    }
  
    VboObject* vboObject;
    unsigned int indexLength;
  
    for(auto i=0; i< spaces.size(); i++) {
      nmath::ImageData<float>* mesh = spaces[i];
      unsigned short* triangleTripIndices = mesh->generateIndices(indexLength);
      const float* vertices = mesh->getData();
      // Get the size of the vertices array
      int dataSize = mesh->vertexListSize();
      int vertexCount = mesh->getVertexCount();
      int stride = mesh->getDimension();
      int normalOffset = mesh->getNormalOffset();
  
      float* colors = new float[vertexCount * 4];
      for(int i=0; i< vertexCount; i++) {
        colors[i * 4] = 0.3f;
        colors[i * 4 + 1] = 0.5f;
        colors[i * 4 + 2] = 0.2f;
        colors[i * 4 + 3] = 1.0f;
      }
  
      // For debugging
      std::cout << "vertexCount: " << vertexCount << std::endl;
      std::cout << "dataSize: " << dataSize << std::endl;
      std::cout << "indexLength: " << indexLength << std::endl;
      std::cout << "normalOffset: " << normalOffset << std::endl;
  
      vboObject = new VboObject(locations, GL_TRIANGLE_STRIP);
      vboObject->initialize(vertices, dataSize, vertexCount, colors, triangleTripIndices, indexLength, normalOffset);
      results.push_back(vboObject);
  
      delete[] triangleTripIndices;
      delete[] colors;
    }
  
    return 0;
}

int generateRoundedCone(float height, float radius, float capRadius, int stacks, int capStacks,
  float red, float green, float blue, float alpha, ShaderVarLocation locations, std::vector<VboObject*> &results) {
  
  // Create a rounded cone mesh
  Cone cone(height, radius, capRadius, stacks, capStacks);
  std::vector<float> vertices;
  std::vector<unsigned short> indices;

  // Generate the cone mesh
  int vertexCount = cone.generateMesh(vertices, indices);

  // Create a color array for the vertices
  float * colors = new float[vertexCount * 4];
  for (int i = 0; i < vertexCount; ++i) {
    colors[i * 4] = red;
    colors[i * 4 + 1] = green;
    colors[i * 4 + 2] = blue;
    colors[i * 4 + 3] = alpha;
  }

  // Create a VboObject for the cone
  VboObject* vboObject = new VboObject(locations, GL_TRIANGLES);
  vboObject->initialize(vertices.data(), vertices.size(), vertexCount, colors, indices.data(), indices.size(), 3);

  results.push_back(vboObject);

  delete[] colors;

  return 0;
}