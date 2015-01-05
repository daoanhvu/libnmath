#ifndef _PROCEDURE_H
#define _PROCEDURE_H

#include <Windows.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501                     // for WM_MOUUSEWHEEL. Is this right?
#endif

namespace Win {
	LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK GraphWinproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK dialogProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

#endif