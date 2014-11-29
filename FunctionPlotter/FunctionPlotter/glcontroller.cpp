#include <process.h>
#include "errorconst.h"
#include "glcontroller.h"

using namespace Win;

GLController::GLController(ModelGL *model, ViewGL *view): mModel(model), mView(view), mThreadHandle(0),
		mThreadId(0), mLoopFlag(false) {
}

int GLController::close() {
	mLoopFlag = false;
	::WaitForSingleObject(mThreadHandle, INFINITE);

	mView->closeContext();
	::DestroyWindow(mHandle);
	return 0;
}

int GLController::init() {
	mView->createWindow(this);
	if (mView->createGLContext(32, 24, 8) != NFP_OK) {
		return -1;
	}
	mThreadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned(__stdcall *)(void*))threadFunction, this, 0, &mThreadId);
	if (mThreadHandle) {
		mLoopFlag = true;
	}
	return 0;
}

int GLController::command(int id, int cmd, LPARAM msg) {
	return 0;
}

/*
	This is callback method and will be call on WM_CREATE of ViewGL::mHandle window control
*/
int GLController::create() {
	return 0;
}

int GLController::paint() {
	PAINTSTRUCT ps;
	::BeginPaint(mView->getHandle(), &ps);
	::EndPaint(mView->getHandle(), &ps);
	return 0;
}

int GLController::size(int w, int h, WPARAM wParam) {
	mView->resizeWindow(w, h);
    return 0;
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
	if(state == MK_LBUTTON) {
		//mModel->rotateCamera(x, y);
	}

	return 0;
}

void GLController::runThread() {
	RECT rect;
	::wglMakeCurrent(mView->getDC(), mView->getRC());

	//IMPORTANT: initialize OpenGL state
	mModel->init();

	::GetClientRect(mHandle, &rect);
	mView->resizeWindow(rect.right, rect.bottom);

	while(mLoopFlag) {
		Sleep(2);		//yield to other processes or threads
		
		mView->resetViewportIfNeeded();
		mModel->draw();
		mView->swapBuffers();
	}

	//terminate rendering thread
	::wglMakeCurrent(0, 0);
	::CloseHandle(mThreadHandle);
}

void GLController::threadFunction(void *arg) {
	((GLController *)arg)->runThread();
}
