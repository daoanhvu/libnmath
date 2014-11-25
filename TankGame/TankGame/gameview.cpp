#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <process.h>

#include "gameview.h"
#include "gamemodel.h"

using namespace glm;
using namespace TankGame;

/*
	http://stackoverflow.com/questions/17779340/glfw-3-0-resource-loading-with-opengl
	https://gist.github.com/Madsy/6980061
*/

GameView::GameView(GameModel *model):mModel(model), mWindow(NULL) {

}

GameView::~GameView(){

}

int GameView::init() {
	if(!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); //4x Antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); //I want to use OpenGL 2.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	mWindow = glfwCreateWindow(800, 600, "TankGame", NULL, NULL);
	if (mWindow == NULL) {
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(mWindow);
	if (glewInit() != GLEW_OK) {
		return 3;
	}
	//Set a background color  
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	do {
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(mWindow));

    return 0;
}

void GameView::start() {
	//mThreadHandle = (HANDLE)_beginthreadex(NULL, NULL, (unsigned (__stdcall *)(void *))startThread, this, 0, &mThreadID);
}

void GameView::runThread() {
	isRunning = true;
	glfwMakeContextCurrent(mWindow);
	if (glewInit() != GLEW_OK) {
		return;
	}
	//Set a background color  
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	do {
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(mWindow);

		glfwPollEvents();
	}while ( isRunning && !glfwWindowShouldClose(mWindow) );
}

/*
	Static routine
*/
void GameView::startThread(void *param) {
	GameView *game = (GameView*)param;
	game->runThread();
}

void GameView::close() {
	isRunning = false;
	::WaitForSingleObject(mThreadHandle, INFINITE);

	glfwMakeContextCurrent(mWindow);
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}