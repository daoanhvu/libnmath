#if _WIN32
#include <Windows.h>
#endif

#include <GL\glew.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "modelgl.h"

using namespace std;

ModelGL::ModelGL(): mProgramID(0), mPositionID(0) {
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;
}

ModelGL::~ModelGL() {
}

void ModelGL::initLights() {
}

void ModelGL::createObject() {
}

void ModelGL::init() {
	glewInit();

	mProgramID = loadShader("fplotter.vertextshader", "fplotter.fragmentshader");
	mPositionID = glGetAttribLocation(mProgramID, "position");
	mEnableLightID = glGetUniformLocation(mProgramID, "enableLight");

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);


}

void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0);
}

void ModelGL::setViewport(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);

	float aspectRatio = (float)width/height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, aspectRatio, 0.1f, 20.0f);
	glMatrixMode(GL_MODELVIEW);
}

void ModelGL::resizeWindow(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	windowResized = true;
}

void ModelGL::draw() {
	if(windowResized) {
		setViewport(windowWidth, windowHeight);
		windowResized = false;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	if(changeDrawMode) {
		switch (drawMode) {
			case 0:
			break;

			case 1:
			break;

			case 2:
			break;
		}
	}

	//Check if background was changing
	if(bgFlag) {
		glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        bgFlag = false;
	}
}

void ModelGL::setDrawMode(int mode) {
	if(drawMode != mode) {
		drawMode = mode;
		changeDrawMode = true;
	}
}
		
void ModelGL::rotateCamera(int x, int y) {
}

void ModelGL::zoomCamera(int dist) {
}

// change background colour
void ModelGL::setBackgroundRed(float value) {
}

void ModelGL::setBackgroundGreen(float value) {
}

void ModelGL::setBackgroundBlue(float value) {
}

GLuint ModelGL::loadShader(const char *vertexShaderFile, const char *fragmentShaderFile) {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLint result;
	int infoLen;

	//Read the Vertex shader
	std::string vertexShaderCode;
	std::ifstream vertexShaderStream(vertexShaderFile, std::ios::in);

	if(!vertexShaderStream.is_open()) {
		return 0;
	}

	std::string line = "";
	while(getline(vertexShaderStream, line)) {
		vertexShaderCode += line + "\n";
	}
	vertexShaderStream.close();

	// Read the Fragment Shader code from the file
	std::string fragmentShaderCode;
	std::ifstream fragmentShaderStream(fragmentShaderFile, std::ios::in);
	if(fragmentShaderStream.is_open()){
		line = "";
		while(getline(fragmentShaderStream, line))
			fragmentShaderCode += line + "\n";
		fragmentShaderStream.close();
	}

	//Compile vertex shader
	const char *vertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
	glCompileShader(vertexShaderID);
	//Check vertex shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLen);
	if ( infoLen > 0 ){
		std::vector<char> vertexShaderErrorMessage(infoLen+1);
		glGetShaderInfoLog(vertexShaderID, infoLen, NULL, &vertexShaderErrorMessage[0]);
		printf("%s\n", &vertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	//printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = fragmentShaderCode.c_str();
	glShaderSource(fragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(fragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLen);
	if ( infoLen > 0 ){
		std::vector<char> fragmentShaderErrorMessage(infoLen+1);
		glGetShaderInfoLog(fragmentShaderID, infoLen, NULL, &fragmentShaderErrorMessage[0]);
		printf("%s\n", &fragmentShaderErrorMessage[0]);
	}

	//Link program
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen);
	if ( infoLen > 0 ){
		std::vector<char> programErrorMessage(infoLen+1);
		glGetProgramInfoLog(programID, infoLen, NULL, &programErrorMessage[0]);
		printf("%s\n", &programErrorMessage[0]);
	}
	
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	
	return programID;
}