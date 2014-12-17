#if _WIN32
#include <Windows.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "modelgl.h"
#include <common.h>

using namespace std;
using namespace nmath;

ModelGL::ModelGL(): mFunction(NULL), mVertexData(0) {
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 255;
	mBackgroundBrush = ::CreateSolidBrush(RGB(bgColor[0], bgColor[1], bgColor[2]));
	//mBackgroundBrush = ::CreateSolidBrush(RGB(100, 40, 250));

	//For testing
	createObject();
}

ModelGL::~ModelGL() {
	release();
}

void ModelGL::release() {
	int i;
	if(mFunction != NULL) {
		delete mFunction;
	}

	if (mVertexData != NULL) {
		for (i = 0; i<mVertexData->size; i++) {
			free(mVertexData->list[i]->data);
			free(mVertexData->list[i]->rowInfo);
			free(mVertexData->list[i]);
		}
		free(mVertexData->list);
		free(mVertexData);
		mVertexData = NULL;
	}

	if (mBackgroundBrush)
		::DeleteObject(mBackgroundBrush);
}

void ModelGL::initLights() {
}

void ModelGL::createObject() {
	float bd[4] = {-2, 2, -2, 2};
	mFunction = new NFunction();
	
	mFunction->parse("f(x,y)=x+cos(y)", 15);
	if(mFunction->getErrorCode() != NMATH_NO_ERROR) {
		delete mFunction;
		return;
	}

	mVertexData = mFunction->getSpace(bd, 0.2f, false);
}

void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
	camera.setup(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0);
	camera.setPerspective(D2R(30), 0.2f, 10);
}

void ModelGL::setViewport(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	camera.setViewport(0, 0, width, height);
}

void ModelGL::resizeWindow(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	windowResized = true;
	camera.setViewport(0, 0, width, height);
	mClientRect.left = 0;
	mClientRect.top = 0;
	mClientRect.right = width;
	mClientRect.bottom = height;
}

void ModelGL::draw(HDC hdc) {
	if(windowResized) {
		setViewport(windowWidth, windowHeight);
		windowResized = false;
	}
	int *indice;
	int indiceSize;
	FillRect(hdc, &mClientRect, mBackgroundBrush);
	HBRUSH brush = CreateSolidBrush(RGB(200,170,20));
	HBRUSH oldPaintRush = (HBRUSH)::SelectObject(hdc, brush);
	
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
	
	if(mVertexData != 0) {
		for(int i=0; i<mVertexData->size; i++) {
			indice = camera.buildIndicesForTriangleStrip(mVertexData->list[i]->dataSize / mVertexData->list[i]->dimension, 
				mVertexData->list[i]->rowInfo, mVertexData->list[i]->rowCount, &indiceSize);
			camera.drawTriangleTrip(hdc, mVertexData->list[i]->data, indice, indiceSize);
			if (indice != NULL)
				delete[] indice;
		}
	}

	//Check if background was changing
	if(bgFlag) {
        bgFlag = false;
	}
	
	::SelectObject(hdc, oldPaintRush);
	::DeleteObject(brush);
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
	mBackgroundBrush = ::CreateSolidBrush(RGB(red,green,blue));
}
