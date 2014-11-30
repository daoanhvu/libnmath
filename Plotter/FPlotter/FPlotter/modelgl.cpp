#if _WIN32
#include <Windows.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "modelgl.h"

using namespace std;

ModelGL::ModelGL(): mFunction(NULL), data(NULL) {
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;
}

ModelGL::~ModelGL() {
}

void ModelGL::release() {
	int i;
	if(mFunction != NULL) {
		releaseFunct(mFunction);
		free(mFunction);
	}

	if(data != NULL) {
		for(i=0; i<data->size; i++) {
			free(data->list[i]->data);
			free(data->list[i]->rowInfo);
			free(data->list[i]);
		}
		free(data->list);
		free(data);
	}
}

void ModelGL::initLights() {
}

void ModelGL::createObject() {
	float bd[4] = {-2, 2, -2, 2};
	mFunction = (Function*)malloc(sizeof(Function));
	mFunction->prefix = NULL;
	mFunction->domain = NULL;
	mFunction->criterias = NULL;
	mFunction->str = NULL;
	mFunction->len = 0;
	mFunction->variableNode = NULL;
	mFunction->numVarNode = 0;
	mFunction->valLen = 0;

	parseFunction("f(x,y)=sin(x)+cos(y)", 20, mFunction);
	if(getErrorCode() != NMATH_NO_ERROR) {
		releaseFunct(mFunction);
		free(mFunction);
		return;
	} 

	data = getSpaces(mFunction, bd, mFunction->valLen * 2, 0.1f);
}

void ModelGL::init() {
}

void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
}

void ModelGL::setViewport(int width, int height) {
	windowWidth = width;
	windowHeight = height;
}

void ModelGL::resizeWindow(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	windowResized = true;
	camera.setViewport(0, 0, width, height);
}

void ModelGL::draw() {
	if(windowResized) {
		setViewport(windowWidth, windowHeight);
		windowResized = false;
	}

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
void ModelGL::setBackgroundColor(BYTE red, BYTE green, BYTE, BYTE blue) {
}
