#ifndef _WINDOW_H
#define _WINDOW_H

#include <windows.h>
#include "controller.h"

namespace Win {
	class Window {
	private:
		int x;
		int y;
		int width;
		int height;

		HWND parentHandle;
		HMENU menuHandle;
		HINSTANCE instance;
		WNDCLASSEX winClass;

		Controller *controller;
	public:
		Window(HINSTANCE hInst, const wchar_t *name, HWND hParent, Controller *ctrl);
		~Window();

		HWND create();
	};
}

#endif