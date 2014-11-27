#define WIN32_LEAN_AND_MEAN             // exclude rarely-used stuff from Windows headers

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include "fplotter.h"
#include "controllermain.h"
#include "glcontroller.h"
#include "window.h"
#include "dialogwindow.h"
#include "modelgl.h"
#include "viewgl.h"
#include "ConsolePaneController.h"

/*
	https://www.opengl.org/wiki/Tutorial:_OpenGL_3.1_The_First_Triangle_%28C%2B%2B/Win%29
*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND activeWindow;
    MSG msg;
	HACCEL hAccelTable = 0;
	wchar_t appName[256];
	HWND statusHandle;

	INITCOMMONCONTROLSEX commonCtrlEx;
	commonCtrlEx.dwSize = sizeof(commonCtrlEx);
	commonCtrlEx.dwICC = ICC_BAR_CLASSES;	//trackbar in this class
	::InitCommonControlsEx(&commonCtrlEx);

	//get App name from resource file
	::LoadString(hInstance, IDS_APP_NAME, appName, 256);

	Win::ControllerMain ctrlMain;
	Win::Window windowMain(hInstance, appName, 0, &ctrlMain);

	windowMain.setMenuName(MAKEINTRESOURCE(IDR_MAIN_MENU));
	windowMain.setSize(800, 650);
	windowMain.setWindowStyleEx(WS_EX_WINDOWEDGE);
	windowMain.create();

	ModelGL modelGL;

	//Register GL window class
	Win::ViewGL viewGL(hInstance, windowMain.getHandle(), L"MyGLWindow", 800, 500, CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 3, 1);
	Win::GLController controllerGL(&modelGL, &viewGL);
	Win::ConsoleView consoleView;

	//Create GL window and init OpenGL
	controllerGL.init();

	Win::ConsolePaneController consolePaneCtrl(&modelGL, &consoleView);

	Win::DialogWindow dlgConsole(hInstance, IDD_CONSOLE, windowMain.getHandle(), &consolePaneCtrl);
	dlgConsole.create();

	statusHandle = ::CreateWindowEx(0, STATUSCLASSNAME, 0, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, windowMain.getHandle(), (HMENU)IDC_STATUSBAR, ::GetModuleHandle(0), 0);

	if(statusHandle) {
        ::SendMessage(statusHandle, SB_SETTEXT, 0, (LPARAM)L"Ready");
	}

	ctrlMain.setGLHandle(viewGL.getHandle());
	ctrlMain.setConsoleHandle(dlgConsole.getHandle());

	viewGL.show();
	dlgConsole.show();
	windowMain.show();

    while(::GetMessage(&msg, NULL, 0, 0) > 0) {
		activeWindow = ::GetActiveWindow();

		if(!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }

    return (int)msg.wParam;
}