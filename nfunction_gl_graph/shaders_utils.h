#ifndef _SHADERS_UTILS_H_
#define _SHADERS_UTILS_H_

#include <string>

enum BUFFER_TYPE {
    ONE_BUFFER,
    POSITION_NORMAL_AND_COLOR
};

typedef struct tagLocation {
  unsigned int positionLocation;
  unsigned int normalLocation;
  unsigned int colorLocation;
  unsigned int textureLocation;
  
  //uniform variables
  unsigned int mvpMatrixId;
  unsigned int perspectiveMatrixId;
  unsigned int viewMatrixId;
  unsigned int viewPosId;
  unsigned int modelMatrixId;
	unsigned int modelViewMatrixId;
  unsigned int normalMatrixId;
  unsigned int useNormalID;
  unsigned int useLightingLocation;
  unsigned int pointSizeLocation;
  unsigned int lightPos1ID;
  unsigned int lightPos2ID;
  unsigned int lightColor1ID;
  unsigned int lightColor2ID;
} ShaderVarLocation;

std::string readShaderFile(const char* filePath);
unsigned int compileShader(const char* source, unsigned int shaderType);

#endif