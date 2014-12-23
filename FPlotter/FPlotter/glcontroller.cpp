#include <process.h>
#include <math.h>
#include "messages.h"
#include "glcontroller.h"

using namespace Win;

GLController::GLController(ModelGL *model): mModel(model), mThreadHandle(0),
		mThreadId(0), mLoopFlag(false), mMouseX(0), mMouseY(0) {
}

void GLController::releaseBuffer() {
	if(hMemoryBitmap != 0) {
		DeleteObject(hMemoryBitmap);
		DeleteDC(memoryDC);

		hMemoryBitmap = NULL;
		memoryDC = NULL;
	}
}

void GLController::invalidate(const RECT* rect, BOOL erase) {
	::InvalidateRect(mHandle, rect, erase);
}

int GLController::close() {
	mLoopFlag = false;
	::WaitForSingleObject(mThreadHandle, INFINITE);

	::DestroyWindow(mHandle);
	return 0;
}

int GLController::create() {
	mModel->setCamera(0, 0, -8.0f, 0, 0, 0);
	/*
	mThreadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall *)(void*))threadFunction, this, 0, &mThreadId);
	if(mThreadHandle) {
		mLoopFlag = true;
	}
	*/
	return 0;
}

int GLController::paint() {
	mHdc = BeginPaint(mHandle, &mPS);
	mModel->draw(memoryDC);
	BitBlt(mHdc, 0, 0, mWidth, mHeight, memoryDC, 0, 0, SRCCOPY);
	EndPaint(mHandle, &mPS);
	return 0;
}

int GLController::size(int w, int h, WPARAM wParam) {
	updateBuffer(w, h);
	mModel->resizeWindow(w, h);
	invalidate(NULL, TRUE);
    return 0;
}

void GLController::updateBuffer(int w, int h) {
	mHdc = ::GetDC(mHandle);
	mWidth = w;
	mHeight = h;
	if(hMemoryBitmap != 0) {
		DeleteObject(hMemoryBitmap);
		DeleteDC(memoryDC);
	}
	memoryDC = ::CreateCompatibleDC(mHdc);
	hMemoryBitmap = CreateCompatibleBitmap(mHdc, w, h);

	SelectObject(memoryDC, hMemoryBitmap);	
	::GetClientRect(mHandle, &mClientRect);
	::ReleaseDC(mHandle, mHdc);
}

int GLController::lButtonDown(WPARAM state, int x, int y) {
	return 0;
}

int GLController::lButtonUp(WPARAM state, int x, int y) {
	return 0;
}

int GLController::rButtonDown(WPARAM state, int x, int y) {
	return 0;
}

int GLController::rButtonUp(WPARAM state, int x, int y) {
	return 0;
}

int GLController::mouseMove(WPARAM state, int x, int y) {
	int dx = x - mMouseX;
	int dy = y - mMouseY;
	float yaw, pitch;
	if(state == MK_LBUTTON && mModel->isRotating()) {
		yaw = angle2DVector(dx, dy, 1, 0);
		pitch = angle2DVector(dx, dy, 0, 1);
		mModel->rotateCamera(yaw, pitch);
		invalidate(NULL, true);
	}

	mMouseX = x;
	mMouseY = y;
	return 0;
}

void GLController::runThread() {
	while(mLoopFlag) {
		Sleep(1);		//yield to other processes or threads
		//Do something here
	}
	
	::CloseHandle(mThreadHandle);
}

void GLController::threadFunction(void *arg) {
	((GLController *)arg)->runThread();
}

int GLController::command(int id, int cmd, LPARAM msg) {   // for WM_COMMAND
	switch(id) {
		//case FPLOTTER_SET_ROTATE:
		//return 0;

		case FPLOTTER_SET_ROTATE:
			mModel->setRotateMode(cmd);
			invalidate(NULL, true);
			break;
	}

	return 0;
}


int GLController::onOtherMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
	case FPLOTTER_SET_ROTATE:
		break;

	default:
		return ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

float Win::angle2DVector(float x1, float y1, float x2, float y2) {
	float d = (x1 * x2) + (y1 * y2);
	float cs = d/(sqrt(x1*x1 + y1*y1) * sqrt(x2*x2 + y2*y2) );
	return acos(cs);
}

glm::vec4 Win::quaternion(glm::vec3 va, float phi) {
	glm::vec3 u = glm::normalize(va) * glm::sin(phi);
	return glm::vec4(u[0], u[1], u[2], glm::cos(phi));
}