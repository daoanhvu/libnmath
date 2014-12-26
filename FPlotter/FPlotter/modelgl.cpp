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

ModelGL::ModelGL() : mFunction(NULL), mVertexData(0),mStatus(0) {
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 255;
	mBackgroundBrush = ::CreateSolidBrush(RGB(bgColor[0], bgColor[1], bgColor[2]));
	//mBackgroundBrush = ::CreateSolidBrush(RGB(100, 40, 250));

	mIndice = 0;
	numOfIndice = 0;

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

	if (mIndice != NULL) {
		for (i = 0; i<numOfIndice; i++)
			delete mIndice[i].indice;

		delete mIndice;
	}

	if (mBackgroundBrush)
		::DeleteObject(mBackgroundBrush);
}

void ModelGL::initLights() {
}

void ModelGL::createObject() {
	float bd[4] = {-2, 2, -2, 2};
	int i;

	mFunction = new NFunction();
	
	mFunction->parse("f(x,y)=x+cos(y)", 15);
	if(mFunction->getErrorCode() != NMATH_NO_ERROR) {
		delete mFunction;
		return;
	}

	mVertexData = mFunction->getSpace(bd, 0.2f, false);
	if (mVertexData != NULL) {
		numOfIndice = mVertexData->size;
		mIndice = new TIndex[numOfIndice];

		for (i = 0; i < mVertexData->size; i++) {
			mIndice[i].indice = camera.buildIndicesForTriangleStrip(mVertexData->list[i]->dataSize / mVertexData->list[i]->dimension,
				mVertexData->list[i]->rowInfo, mVertexData->list[i]->rowCount, &(mIndice[i].size));
		}
	}
}

void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ) {
	camera.setup(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0);
	//camera.setPerspective(D2R(30), 0.2f, 10);
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
	camera.setPerspective(D2R(30), 0.2f, 10);
	mClientRect.left = 0;
	mClientRect.top = 0;
	mClientRect.right = width;
	mClientRect.bottom = height;
}

void ModelGL::draw(HDC hdc) {
	HPEN hEllipsePen, hPenOld;
	COLORREF qEllipseColor;
	qEllipseColor = RGB(0, 0, 255);
	/*
	if(windowResized) {
		setViewport(windowWidth, windowHeight);
		windowResized = false;
	}
	*/
	
	FillRect(hdc, &mClientRect, mBackgroundBrush);
	//HBRUSH brush = CreateSolidBrush(RGB(200,170,20));
	//HBRUSH oldPaintRush = (HBRUSH)::SelectObject(hdc, brush);
	
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
			camera.drawTriangleTrip(hdc, mVertexData->list[i]->data, mIndice[i].indice, mIndice[i].size);
		}
	}

	if(mStatus == 1) {
		hEllipsePen = CreatePen(PS_SOLID, 1, qEllipseColor);
		hPenOld = (HPEN)SelectObject(hdc, hEllipsePen);
		//Ellipse(hdc, windowWidth/2-15, windowHeight/2 - 60, windowWidth/2+15, windowHeight/2+60);
		Arc(hdc, windowWidth/2-15, windowHeight/2 - 100, windowWidth/2+15, windowHeight/2+100, 0, 0, 0, 0);
		Arc(hdc, windowWidth/2-100, windowHeight/2 - 15, windowWidth/2+100, windowHeight/2+15, 0, 0, 0, 0);
		SelectObject(hdc, hPenOld);
		DeleteObject(hEllipsePen);
	}

	//Check if background was changing
	if(bgFlag) {
        bgFlag = false;
	}
	
	//::SelectObject(hdc, oldPaintRush);
	//::DeleteObject(brush);
}

void ModelGL::setDrawMode(int mode) {
	if(drawMode != mode) {
		drawMode = mode;
		changeDrawMode = true;
	}
}
		
void ModelGL::rotateCamera(float yaw, float pitch) {
	camera.rotate(yaw, pitch, 0);
}

void ModelGL::zoomCamera(int dist) {
}

// change background colour
void ModelGL::setBackgroundColor(BYTE red, BYTE green, BYTE, BYTE blue) {
	mBackgroundBrush = ::CreateSolidBrush(RGB(red,green,blue));
}
