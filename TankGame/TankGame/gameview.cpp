#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <process.h>
#include <string>
#include <fstream>
#include <vector>

#include "gameview.h"
#include "gamemodel.h"

using namespace glm;
using namespace TankGame;

/*
	http://stackoverflow.com/questions/17779340/glfw-3-0-resource-loading-with-opengl
	https://gist.github.com/Madsy/6980061
*/

GameView::GameView(GameModel *model): mProgramId(0), mModel(model), mWindow(NULL) {
}

GameView::~GameView(){

}

int GameView::init(int vmajor, int vminor) {
	if(!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); //4x Antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, vmajor); //I want to use OpenGL 2.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, vminor);

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

	/*	At developing time, I use absolute path
		mProgramId = loadShaders("tank1.vertexshader", "tank1.fragmentshader");
	*/
	mProgramId = loadShaders("D:\\projects\\libnmath\\TankGame\\TankGame\\shaders\\vertex.glsl",
		"D:\\projects\\libnmath\\TankGame\\TankGame\\shaders\\fragment.glsl");
	if (mProgramId <= 0) {
		glfwTerminate();
		return 4;
	}

	mPositionLocation = glGetAttribLocation(mProgramId, "position");
	mInColorLocation = glGetAttribLocation(mProgramId, "inColor");

	if (mModel != NULL){
		mModel->init();
	}

    return 0;
}

void GameView::start() {

	//mThreadHandle = (HANDLE)_beginthreadex(NULL, NULL, (unsigned (__stdcall *)(void *))startThread, this, 0, &mThreadID);

	do {
		glUseProgram(mProgramId);
		glClear(GL_COLOR_BUFFER_BIT);
		//Draw stuffs here
		mModel->render();
		glUseProgram(0);

		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(mWindow));
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

	//glfwMakeContextCurrent(mWindow);
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

/*
	This method return ProgramID on success
*/
GLuint GameView::loadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLint result = GL_FALSE;
	int inforLen;
	GLuint programID;
	std::string line = "";

	// Read the Vertex Shader code from the file
	std::string vertexShaderCode;
	std::ifstream vertexShaderStream(vertex_file_path, std::ios::in);
	if (vertexShaderStream.is_open()) {
		line = "";
		while (getline(vertexShaderStream, line))
			vertexShaderCode += line + "\n";
		vertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		line = "";
		while (getline(FragmentShaderStream, line))
			FragmentShaderCode += line + "\n";
		FragmentShaderStream.close();
	}

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(vertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &inforLen);
	std::vector<char> VertexShaderErrorMessage(inforLen);
	glGetShaderInfoLog(vertexShaderID, inforLen, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(fragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &inforLen);
	std::vector<char> FragmentShaderErrorMessage(inforLen);
	glGetShaderInfoLog(fragmentShaderID, inforLen, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &inforLen);
	std::vector<char> ProgramErrorMessage(max(inforLen, int(1)));
	glGetProgramInfoLog(programID, inforLen, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}