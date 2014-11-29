#if _WIN32
#include <Windows.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "modelgl.h"

using namespace std;

ModelGL::ModelGL() {
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;
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
}

void ModelGL::setViewport(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	

	float aspectRatio = (float)width/height;
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
