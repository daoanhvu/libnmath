#include <process.h>
#include "glcontroller.h"

using namespace Win;

GLController::GLController(ModelGL *model, ViewGL *view): mModel(model), mView(view), mThreadHandle(0),
		mThreadId(0), mLoopFlag(false) {
}

int GLController::close() {
	mLoopFlag = false;
	::WaitForSingleObject(mThreadHandle, INFINITE);

	mView->closeContext(mHandle);
	::DestroyWindow(mHandle);
	return 0;
}

int GLController::command(int id, int cmd, LPARAM msg) {
	return 0;
}

int GLController::create() {
	if(!mView->createContext(mHandle, 32, 24, 8))
		return -1;

	mThreadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall *)(void*))threadFunction, this, 0, &mThreadId);
	if(mThreadHandle) {
		mLoopFlag = true;
	}

	return 0;
}

int GLController::paint() {
	return 0;
}

int GLController::size(int w, int h, WPARAM wParam) {
	mModel->resizeWindow(w, h);
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
	mModel->resizeWindow(rect.right, rect.bottom);

	while(mLoopFlag) {
		Sleep(1);		//yield to other processes or threads
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
