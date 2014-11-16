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
    HWND hwnd;
    MSG msg;
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

	/*
	hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        gSzClassName,
        "Function Plotter v1.0",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
        NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
	*/

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}