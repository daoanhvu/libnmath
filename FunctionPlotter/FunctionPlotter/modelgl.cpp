#if _WIN32
#include <Windows.h>
#endif

#include <GL\glew.h>
#include "modelgl.h"

ModelGL::ModelGL() {
	//bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;
	bgColor[0] = bgColor[1] = bgColor[3] = 0;
	bgColor[2] = 1.0f;
}

ModelGL::~ModelGL() {
}

void ModelGL::initLights() {
}

void ModelGL::createObject() {
}

void ModelGL::init() {
}

void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0);
}

void ModelGL::draw() {
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
