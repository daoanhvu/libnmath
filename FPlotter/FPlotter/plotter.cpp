#define WIN32_LEAN_AND_MEAN             // exclude rarely-used stuff from Windows headers
#define GLM_FORCE_RADIANS

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include "procedure.h"
#include "controllermain.h"
#include "glcontroller.h"
#include "window.h"
#include "dialogwindow.h"
#include "modelgl.h"
#include "ConsolePaneController.h"

HWND initToolbar(HINSTANCE hInstance, HWND hWindow, TBBUTTON *tbrButtons, int numOfButton) {
	HWND hToolbar;

	hToolbar = CreateToolbarEx(hWindow, WS_VISIBLE | WS_CHILD | WS_BORDER,
		IDR_TB_MAIN,
	    numOfButton,
		hInstance,
		IDR_TB_MAIN,
		tbrButtons,
		numOfButton,
		16, 16, 16, 16,
		sizeof(TBBUTTON));

	return hToolbar;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND activeWindow;
    MSG msg;
	HACCEL hAccelTable = 0;
	wchar_t appName[256];
	HWND statusHandle;
	HWND hToolbar;
	TBBUTTON tbrButtons[3];

	INITCOMMONCONTROLSEX commonCtrlEx;
	commonCtrlEx.dwSize = sizeof(commonCtrlEx);
	commonCtrlEx.dwICC = ICC_BAR_CLASSES;	//trackbar in this class
	::InitCommonControlsEx(&commonCtrlEx);

	//get App name from resource file
	::LoadString(hInstance, IDS_APP_NAME, appName, 256);

	Win::ControllerMain ctrlMain;
	Win::Window windowMain(hInstance, appName, 0, Win::windowProcedure, &ctrlMain);
	ctrlMain.setWindow(&windowMain);

	windowMain.setMenuName(MAKEINTRESOURCE(IDR_MAIN_MENU));
	windowMain.setSize(800, 650);
	windowMain.setWindowStyleEx(WS_EX_WINDOWEDGE);
	windowMain.create();
	
	//Toolbar
	tbrButtons[0].iBitmap   = 0;
	tbrButtons[0].idCommand = ID_TBB_ADD_F1;
	tbrButtons[0].fsState   = TBSTATE_ENABLED;
	tbrButtons[0].fsStyle   = TBSTYLE_BUTTON;
	tbrButtons[0].dwData    = 0L;
	tbrButtons[0].iString   = 0;
	
	tbrButtons[1].iBitmap   = 1;
	tbrButtons[1].idCommand = ID_TBB_BACKGROUND;
	tbrButtons[1].fsState   = TBSTATE_ENABLED;
	tbrButtons[1].fsStyle   = TBSTYLE_BUTTON;
	tbrButtons[1].dwData    = 0L;
	tbrButtons[1].iString   = 0;

	tbrButtons[2].iBitmap   = 2;
	tbrButtons[2].idCommand = ID_TBB_ROTATE;
	tbrButtons[2].fsState   = TBSTATE_ENABLED;
	tbrButtons[2].fsStyle   = TBSTYLE_CHECK;
	tbrButtons[2].dwData    = 0L;
	tbrButtons[2].iString   = 0;
	hToolbar = initToolbar(hInstance, windowMain.getHandle(), tbrButtons, 3);


	ModelGL modelGL;
	Win::GLController controllerGL(&modelGL);
	Win::Window glChildWindow(hInstance, L"MyGLWindow", windowMain.getHandle(), Win::GraphWinproc, &controllerGL);

	glChildWindow.setClassStyle(CS_OWNDC);
	glChildWindow.setWindowStyle(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	glChildWindow.setSize(800, 550);
	glChildWindow.create();

	Win::ConsoleView consoleView;
	Win::ConsolePaneController consolePaneCtrl(&modelGL, &consoleView);
	Win::DialogWindow dlgConsole(hInstance, IDD_CONSOLE, windowMain.getHandle(), &consolePaneCtrl);
	dlgConsole.create();

	statusHandle = ::CreateWindowEx(0, STATUSCLASSNAME, 0, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, windowMain.getHandle(), (HMENU)IDC_STATUSBAR, ::GetModuleHandle(0), 0);

	if(statusHandle) {
        ::SendMessage(statusHandle, SB_SETTEXT, 0, (LPARAM)L"Ready");
	}

	ctrlMain.setDrawingHandle(glChildWindow.getHandle());
	ctrlMain.setConsoleHandle(dlgConsole.getHandle());

	glChildWindow.show();
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