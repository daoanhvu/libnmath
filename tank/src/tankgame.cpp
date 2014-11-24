#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

#include "tankgame.h"
#include "gamecontroller.h"
#include "gameview.h"
#include "gamemodel.h"

using namespace glm;

int main(int argc, char* args[]) {

	if(!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLE, 4); //4x Antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); //I want to use OpenGL 2.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	


	return 0;
}