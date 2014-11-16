#define WIN32_LEAN_AND_MEAN             // exclude rarely-used stuff from Windows headers

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include "controllermain.h"
#include "window.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wce;
    HWND hwnd, activeWindow;
    MSG msg;
	HACCEL hAccelTable = 0;
	wchar_t appName[256];

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



	if(!RegisterClassEx(&wce)) {
		MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
	    return 0;
	}

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);


    while(::GetMessage(&msg, NULL, 0, 0) > 0) {
		activeWindow = ::GetActiveWindow();

		if(!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }

    return msg.wParam;
}