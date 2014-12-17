#include "viewgl.h"
#include "resource.h"

using namespace Win;

ViewGL::ViewGL(void): mHdc(0), hMemoryBitmap(0) {
}


ViewGL::~ViewGL(void) {
	if(hMemoryBitmap != 0) {
		DeleteObject(hMemoryBitmap);
		DeleteDC(memoryDC);
	}
}

void ViewGL::releaseBuffer() {
	if(hMemoryBitmap != 0) {
		DeleteObject(hMemoryBitmap);
		DeleteDC(memoryDC);

		hMemoryBitmap = NULL;
		memoryDC = NULL;
	}
}

/*
bool ViewGL::setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits) {
	PIXELFORMATDESCRIPTOR pfd;
	int pixelFormat = findPixelFormat(hdc, colorBits, depthBits, tencilBits);
	if(pixelFormat == 0)
		return false;

	::DescribePixelFormat(hdc, pixelFormat, sizeof(pfd), &pfd);

	if(!::SetPixelFormat(hdc, pixelFormat, &pfd)) {
		return false;
	}

	return true;
}
*/

void ViewGL::updateBuffer(HWND handle, int w, int h) {
	mHdc = ::GetDC(handle);
	mWidth = w;
	mHeight = h;
	if(hMemoryBitmap != 0) {
		DeleteObject(hMemoryBitmap);
		DeleteDC(memoryDC);
	}
	memoryDC = ::CreateCompatibleDC(mHdc);
	hMemoryBitmap = CreateCompatibleBitmap(mHdc, w, h);

	SelectObject(memoryDC, hMemoryBitmap);	
	::GetClientRect(handle, &mClientRect);
	::ReleaseDC(handle, mHdc);
}

void ViewGL::paint(HWND handle, ModelGL* model){
	mHdc = BeginPaint(handle, &mPS);
	model->draw(memoryDC);
	BitBlt(mHdc, 0, 0, mWidth, mHeight, memoryDC, 0, 0, SRCCOPY);
	EndPaint(handle, &mPS);
}