#include <Windows.h>
#include <CommCtrl.h>
#include <sstream>
#include "controllermain.h"
#include "NewFunctionDlgCtrl.h"
#include "messages.h"
#include "resource.h"

using namespace Win;

// handle events(messages) on all child windows that belong to the parent window.
// For example, close all child windows when the parent got WM_CLOSE message.
// lParam can be used to specify a event or message.
bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam);


ControllerMain::ControllerMain():mDrawingHandle(0), consolePaneHandle(0), dialog(0), mBttRotateStatus(false) {
}

ControllerMain::~ControllerMain() {
	Controller *ctrl;
	if(dialog != NULL) {
		ctrl = dialog->getController();
		delete ctrl;
		delete dialog;
	}
}

int ControllerMain::command(int id, int cmd, LPARAM msg) {   // for WM_COMMAND
	
	/* Should use switch struct here if you have many case of command id */
	Win::NewFunctionDlgCtrl* consolePaneCtrl;

	switch(id) {
	case ID_FILE_EXIT:
			::PostMessage(mHandle, WM_CLOSE, 0, 0);
		return 0;

		case ID_TBB_ADD_F1:
			if(dialog == NULL) {
				consolePaneCtrl = new NewFunctionDlgCtrl();
				dialog = new DialogWindow(mWindow->getInstance(), IDD_INPUT_FUNCTION1, NULL, consolePaneCtrl);
				dialog->create();
				dialog->show();
			}
		return 0;

		case ID_TBB_BACKGROUND:
		
		return 0;

		case ID_TBB_ROTATE:
			mBttRotateStatus = !mBttRotateStatus;
			WPARAM wParam = MAKEWPARAM( FPLOTTER_SET_ROTATE, (mBttRotateStatus?1:0) );
			BOOL bval = ::PostMessage(mDrawingHandle, WM_COMMAND, wParam, (LPARAM)0);
			//::SendMessage(mDrawingHandle, FPLOTTER_SET_ROTATE, 0, (LPARAM)(mBttRotateStatus?1:0));
			break;
	}

	return 0;
}

int ControllerMain::create() {    // for WM_CRERATE
	return 0;
}

int ControllerMain::close() {
	//Close all child windows first
	::EnumChildWindows(mHandle, (WNDENUMPROC)enumerateChildren, (LPARAM)WM_CLOSE);
	::DestroyWindow(mHandle);
	return 0;
}

int ControllerMain::destroy() {
	::PostQuitMessage(0);
	return 0;
}

int ControllerMain::size(int width, int height, WPARAM wParam) {      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)
	RECT rect;
	int statusHeight, consolePaneHeight, glHeight;
	//Get height of status bar
	HWND statusHandle = ::GetDlgItem(mHandle, IDC_STATUSBAR);
	::GetWindowRect(statusHandle, &rect);
	statusHeight = rect.bottom - rect.top;
	
	//Get height of the console pane
	::GetWindowRect(consolePaneHandle, &rect);
	consolePaneHeight = rect.bottom - rect.top;
	
	//Resize the height of glWin
	glHeight = height - consolePaneHeight - statusHeight;
	::SetWindowPos(mDrawingHandle, 0, 0, 0, width, glHeight, SWP_NOZORDER);
	::SetWindowPos(consolePaneHandle, 0, 0, glHeight, width, consolePaneHeight, SWP_NOZORDER);
	::InvalidateRect(consolePaneHandle, 0, TRUE);
	::SendMessage(statusHandle, WM_SIZE, 0, 0); //automatically resize width so send 0s
	::InvalidateRect(statusHandle, 0, FALSE);
	
	
	// display OpenGL window dimension on the status bar
    std::wstringstream wss;
    wss << "Window Size (Client Area): " << width << " x " << height;
    ::SendMessage(statusHandle, SB_SETTEXT, 0, (LPARAM)wss.str().c_str());

	return 0;
}

bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam) {
	if(lParam == WM_CLOSE)
		::SendMessage(childHandle, WM_CLOSE, 0, 0);
	return true;
}