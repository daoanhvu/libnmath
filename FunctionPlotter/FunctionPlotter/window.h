#ifndef _WINDOW_H
#define _WINDOW_H

#include <Windows.h>
#include "controller.h"

namespace Win {
	class Window {
	private:
		enum { MAX_STRING = 256 };              // local constants, max length of string
		wchar_t title[MAX_STRING];
		wchar_t className[MAX_STRING];
		
		int x;
		int y;
		int width;
		int height;

		HWND handle;
		HWND parentHandle;
		HMENU menuHandle;
		HINSTANCE instance;
		WNDCLASSEX winClass;
		DWORD winStyle;
		DWORD winStyleEx;
		Controller *controller;
		
//		void registerClass();
		HICON loadIcon(int id);
		HCURSOR loadCursor(int id);
	public:
		Window(HINSTANCE hInst, const wchar_t *name, HWND hParent, Controller *ctrl);
		~Window();

		HWND create();
		void show(int shw = SW_SHOWDEFAULT);
		HWND getHandle()						{ return handle; }
		
		void setClassStyle(UINT style)			{ winClass.style = style; }
		void setBackgroundColor(int color)		{ winClass.hbrBackground = (HBRUSH)::GetStockObject(color);}
		void setIcon(int id);
		void setIconSmall(int id);
		void setCursor(int id);
		void setMenuName(LPCTSTR name)			{ winClass.lpszMenuName = name; }
		
		void setWindowStyle(DWORD style)		{ winStyle = style; }
		void setWindowStyleEx(DWORD style)		{ winStyleEx = style; }
		void setPosition(int x, int y)			{ this->x = x; this->y = y; }
		void setWidth(int w)					{ width = w; }
		void setHeight(int h)					{ height = h; }
		void setSize(int w, int h)				{ width = w; height = h; }
		void setParent(HWND hwnd)				{ parentHandle = hwnd; }
		void setMenu(HMENU menu)				{ menuHandle = menu; }
	};
}

#endif