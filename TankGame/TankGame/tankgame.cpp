#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>

#include "tankgame.h"
#include "gamecontroller.h"
#include "gameview.h"
#include "gamemodel.h"

using namespace glm;
using namespace TankGame;

int main(int argc, char* args[]) {

	GameModel gameModel;
	GameView gameView(&gameModel);
	GameController gameController(&gameModel, &gameView);

	if (gameController.init() == 0) {
		gameController.start();
		gameController.stop();
	}

	return 0;
}

int startNormal() {
	GLFWwindow *window;

	if (!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); //4x Antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); //I want to use OpenGL 2.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	window = glfwCreateWindow(800, 600, "TankGame", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		return 1;
	}

	//Set a background color  
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	do{
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();

	} while (!glfwWindowShouldClose(window));

	glfwDestroyWindow(window);
	glfwTerminate();
}