#include "controllermain.h"

using namespace Win;

// handle events(messages) on all child windows that belong to the parent window.
// For example, close all child windows when the parent got WM_CLOSE message.
// lParam can be used to specify a event or message.
bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam);


ControllerMain::ControllerMain():glHandle(0) {
}

int ControllerMain::command(int id, int cmd, LPARAM msg) {   // for WM_COMMAND
	
	/* Should use switch struct here if you have many case of command id */

	if(id == ID_FILE_EXIT) {
		::PostMessage(handle, WM_CLOSE, 0, 0);
		return 0;
	}
	
}

int ControllerMain::create() {    // for WM_CRERATE
	return 0;
}

int ControllerMain::close() {
	//Close all child windows first
	::EnumChildWindows(handle, (WNDENUMPROC)enumerateChildren, (LPARAM)WM_CLOSE);
	::DestroyWindow(handle);
	return 0;
}

int ControllerMain::destroy() {
	::PostQuitMessage(0);
	return 0;
}

int ControllerMain::size(int w, int h, WPARAM wParam) {      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)
	RECT rect;
	int statusHeight, consolePaneHeight, glHeight;
	//Get height of status bar
	HWND statusHandle = ::GetDlgItem(handle, IDC_STATUSBAR);
	::GetWindowRect(statusHandle, &rect);
	statusHeight = rect.bottom - rect.top;
	
	//Get height of the console pane
	::GetWindowRect(consolePaneHandler, &rect);
	consolePaneHeight = rect.bottom - rect.top;
	
	//Resize the height of glWin
	glHeight = height - consolePaneHeight - statusHeight;
	::SetWindowPos(glHandle, 0, 0, 0, width, glHeight, SWP_NOZORDER);
	::SetWindowPos(consolePaneHandle, 0, 0, glHeight, width, consolePaneHeight, SWP_NOZORDER);
	::InvalidateRect(consolePaneHandle, 0, TRUE);
	::SendMessage(statusHandle, WM_SIZE, 0, 0); //automatically resize width so send 0s
	::InvalidateRect(statusHandle, 0, FALSE);
	
	/*
	// display OpenGL window dimension on the status bar
    std::wstringstream wss;
    wss << "Window Size (Client Area): " << width << " x " << height;
    ::SendMessage(statusHandle, SB_SETTEXT, 0, (LPARAM)wss.str().c_str());
	*/
}

bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam) {
	if(lParam == WM_CLOSE)
		::SendMessage(childHandle, WM_CLOSE, 0, 0);
	return true;
}