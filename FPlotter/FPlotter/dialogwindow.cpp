#include <sstream>
#include <iostream>
#include <cstring>
#include "dialogwindow.h"
#include "procedure.h"

using namespace Win;

DialogWindow::DialogWindow(HINSTANCE inst, WORD id, HWND parent, Controller *ctrl):
mHandle(0), mInstance(inst), mId(id), mParentHandle(parent), mController(ctrl), mX(0), mY(0) {
}

HWND DialogWindow::create() {
	mHandle = ::CreateDialogParam(mInstance, MAKEINTRESOURCE(mId), mParentHandle, Win::dialogProcedure, (LPARAM)mController);
	return mHandle;
}

void DialogWindow::show(int cmdShow) {
	::ShowWindow(mHandle, cmdShow);
	::UpdateWindow(mHandle);
}


DialogWindow::~DialogWindow(void) {
}
