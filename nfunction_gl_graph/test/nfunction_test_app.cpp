#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
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
#include "command_handler.h"
#include "windows_helper.h"

int main(int argc, char** argv) {
    // Set up vertex data (and buffer(s)) and configure vertex attributes
    ShaderVarLocation locations;
    // Initialize command handler
    CommandHandler commandHandler(locations);
    
  return 0;
}