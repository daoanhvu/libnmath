#ifndef _PROCEDURE_H
#define _PROCEDURE_H

#include <Windows.h>

namespace Win {
	LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

#endif