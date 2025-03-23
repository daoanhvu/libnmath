#ifndef _SHADERS_UTILS_H_
#define _SHADERS_UTILS_H_

#include <string>
#include <GL/glew.h>

enum BUFFER_TYPE {
    ONE_BUFFER,
    POSITION_NORMAL_AND_COLOR
};

typedef struct tagLocation {
  GLuint positionLocation;
  GLuint normalLocation;
  GLuint colorLocation;
  GLuint textureLocation;
  
  //uniform variables
  GLuint mvpMatrixId;
  GLuint perspectiveMatrixId;
  GLuint viewMatrixId;
  GLuint viewPosId;
  GLuint modelMatrixId;
	GLuint modelViewMatrixId;
  GLuint normalMatrixId;
  GLuint useNormalID;
  GLuint useLightingLocation;
  GLuint pointSizeLocation;
  GLuint lightPos1ID;
  GLuint lightPos2ID;
  GLuint lightColor1ID;
  GLuint lightColor2ID;
} ShaderVarLocation;

std::string readShaderFile(const char* filePath);
GLuint compileShader(const char* source, GLenum shaderType);

#endif