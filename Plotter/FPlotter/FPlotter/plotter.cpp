#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
//#include <GL/glew.h>
//#include <GL/freeglut.h>

#include <common.h>
#include <nlabparser.h>
#include <nmath.h>

LRESULT CALLBACK MessageProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

const char gSzClassName[] = "FunctionPlotter";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wce;
    HWND hwnd;
    MSG msg;

    wce.cbSize        = sizeof(WNDCLASSEX);
    wce.style         = 0;
    wce.lpfnWndProc   = MessageProcedure;
    wce.cbClsExtra    = 0;
    wce.cbWndExtra    = 0;
    wce.hInstance     = hInstance;
    wce.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wce.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wce.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wce.lpszMenuName  = NULL;
    wce.lpszClassName = gSzClassName;
    wce.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wce)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
	    return 0;
	}

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

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK MessageProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
		break;

		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;

		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}