#include "window.h"

Window::Window(HINSTANCE hInst, const wchar_t *name, HWND hParent, Controller *ctrl):handle(0), instance(hInst), controller(ctrl),
	winStyle(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN), windStyleEx(WS_EX_CLIENTEDGE), x(CW_USEDEFAULT), y(CW_USEDEFAULT),
	width(CW_USEDEFAULT), height(CW_USEDEFAULT), parentHandle(hParent), menuHandle(0){
	
	//copy class name and window name
	cwsncpy(this->title, name, MAX_STRING - 1);
	cwsncpy(this->className, name, MAX_STRING - 1);
	
	//populate window class struct
	winClass.cbSize 	   = sizeof(WNDCLASSEX);
	winClass.style         = 0;                                     // class styles: CS_OWNDC, CS_PARENTDC, CS_CLASSDC, CS_GLOBALCLASS, ...
    winClass.lpfnWndProc   = Win::windowProcedure;                  // pointer to window procedure
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;
    winClass.hInstance     = instance;                              // owner of this class
    winClass.hIcon         = LoadIcon(instance, IDI_APPLICATION);   // default icon
    winClass.hIconSm       = 0;
    winClass.hCursor       = LoadCursor(0, IDC_ARROW);              // default arrow cursor
    winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);   // default white brush
    winClass.lpszMenuName  = 0;
    winClass.lpszClassName = className;
    winClass.hIconSm       = LoadIcon(instance, IDI_APPLICATION);   // default small icon
}

Window::~Window() {
	::UnregisterClass(className, instance);
}

HWND Window::create() {
	if(!::RegisterClassEx(&winClass)) return 0;
	
	handle = ::CreateWindowEx(winStyleEx, className, title, winStyle, x, y, 
						width, height, parentHandle, menuHandle, instance, (LPVOID)controller);
	
	return handle;
}

void Window::show(int shw) {
	::ShowWindow(handle, shw);
	::UpdateWindow(handle);
}

HICON Window::loadIcon(int id) {
	return (HICON) ::LoadImage(instance, MAKEINRESOURCE(id), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
}

HCURSOR Window::loadCursor(int id) {
	return (HCURSOR)::LoadImage(instance, MAKEINRESOURCE(id), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
}

void Window::setIcon(int id) {
	winClass.hIcon = (HICON) ::LoadImage(instance, MAKEINRESOURCE(id), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
}

void Window::setIconSmall(int id) {
	winClass.hIconSm = (HICON) ::LoadImage(instance, MAKEINRESOURCE(id), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
}

void Window::setCursor(int id) {
	winClass.hCursor = (HCURSOR)::LoadImage(instance, MAKEINRESOURCE(id), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
}