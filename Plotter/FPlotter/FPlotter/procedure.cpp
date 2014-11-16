
#include "procedure.h"
#include "controller.h"

LRESULT CALLBACK Win::windowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	LRESULT returnVal = 0;

	/* Find the control associates with this window handle */
	static Win::Controller *ctrl;
	
	if(msg == WM_NCCREATE) {
		/*
			Message WM_NCCREATE is called before non-client parts (borders, titlebar, menu..etc)
			are created. This message comes with a pointer to CREATESTRUCT in lParam. The lpCreateParams
			member of CREATESTRUCT actually contains value of the lParam of CreateWindowEx().
		*/
		ctrl = (Controller*)(((CREATESTRUCT*)lparam)->lpCreateParams);
		ctrl->setHandle(hwnd);

		/* After get Controller, we store it into GWLP_USERDATA */
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctrl);

		return ::DefWindowProc(hwnd, msg, wparam, lparam);
	}

	ctrl = (Controller*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if(!ctrl)
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

