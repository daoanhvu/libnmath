#ifndef _FUNCTION_UTILS_H_
#define _FUNCTION_UTILS_H_

#include <vector>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "shaders_utils.h"
#include "vbo_objects.h"
#include "common.hpp"
#include "imagedata.hpp"
#include "nlablexer.h"
#include "nfunction.hpp"
#include "SimpleCriteria.hpp"

int parseCommand(std::string commandText);

int generateMeshAndIndices(std::string inputFunction, const float *values, float epsilon, 
  ShaderVarLocation locations, std::vector<VboObject*> &results);

int generateRoundedCone(float height, float radius, float capRadius, int stacks, int capStacks,
  float red, float green, float blue, float alpha, ShaderVarLocation locations, std::vector<VboObject*> &results);
#endif