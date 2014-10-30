#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
//#include <GL/glew.h>
//#include <GL/freeglut.h>

#include <common.h>
#include <nlabparser.h>
#include <nmath.h>

LRESULT CALLBACK MessageProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow) {

	return 0;
}

LRESULT CALLBACK MessageProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		case WM_DESTROY:
			PostQuitMessage();
		break;
	}
}