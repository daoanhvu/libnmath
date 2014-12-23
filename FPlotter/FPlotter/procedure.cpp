
#include "procedure.h"
#include "controller.h"

LRESULT CALLBACK Win::windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT returnVal = 0;

	/* Find the control associates with this window handle */
	static Win::Controller *ctrl;
	
	if(msg == WM_NCCREATE) {
		/*
			Message WM_NCCREATE is called before non-client parts (borders, titlebar, menu..etc)
			are created. This message comes with a pointer to CREATESTRUCT in lParam. The lpCreateParams
			member of CREATESTRUCT actually contains value of the lParam of CreateWindowEx().
		*/
		ctrl = (Controller*)(((CREATESTRUCT*)lParam)->lpCreateParams);
		ctrl->setHandle(hwnd);

		/* After get Controller, we store it into GWLP_USERDATA */
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctrl);

		return ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	ctrl = (Controller*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
		case WM_CREATE:
			returnVal = ctrl->create();
			break;

		case WM_SIZE:
			//width, height, type
			returnVal = ctrl->size(LOWORD(lParam), HIWORD(lParam), (int)wParam);
			break;

		case WM_ENABLE:
			returnVal = ctrl->enable(wParam!=0);
			break;

		case WM_PAINT:
			ctrl->paint();
			returnVal = ::DefWindowProcA(hwnd, msg, wParam, lParam);
			break;

		case WM_COMMAND:
			returnVal = ctrl->command(LOWORD(wParam), HIWORD(wParam), lParam); //id, code, msg
			break;

		case WM_CLOSE:
			returnVal = ctrl->close();
			break;

		case WM_DESTROY:
			returnVal = ctrl->destroy();
			break;

		case WM_SYSCOMMAND:
			//returnValue = ctrl->sysCommand(wParam, lParam);
			returnVal = ::DefWindowProc(hwnd, msg, wParam, lParam);
		/*
				// Disable Alt-F4 and screensavers
				switch (wparam & 0xfff0)
				{
				case SC_CLOSE:
				case SC_SCREENSAVE:
					ret = 0;
					break;

				default:
					returnValue = DefWindowProc(hwnd, message, wparam, lparam);
					break;
				}
				break;
		*/
			break;

			case WM_CHAR:
        //returnValue = ctrl->char(wParam, lParam);   // route keycode
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        returnVal = ctrl->keyDown((int)wParam, lParam);                       // keyCode, keyDetail
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        returnVal = ctrl->keyUp((int)wParam, lParam);                         // keyCode, keyDetail
        break;

    case WM_LBUTTONDOWN:
        returnVal = ctrl->lButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        //returnValue = ctrl->lButtonDown(wParam, GET_X_LPARAM(lParam), GET_X_LPARAM(lParam)); // state, x, y
        break;

    case WM_LBUTTONUP:
        returnVal = ctrl->lButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_RBUTTONDOWN:
        returnVal = ctrl->rButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        break;

    case WM_RBUTTONUP:
        returnVal = ctrl->rButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_MBUTTONDOWN:
        returnVal = ctrl->mButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        break;

    case WM_MBUTTONUP:
        returnVal = ctrl->mButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_MOUSEHOVER:
        returnVal = ctrl->mouseHover((int)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, x, y
        break;

    case WM_MOUSELEAVE:
        returnVal = ctrl->mouseLeave();
        break;

    case WM_MOUSEMOVE:
        returnVal = ctrl->mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));  // state, x, y
        break;

    case WM_MOUSEWHEEL:
        returnVal = ctrl->mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam)/WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, delta, x, y
        break;

    case WM_HSCROLL:
        returnVal = ctrl->hScroll(wParam, lParam);
        break;

    case WM_VSCROLL:
        returnVal = ctrl->vScroll(wParam, lParam);
        break;

    case WM_TIMER:
        returnVal = ctrl->timer(LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_NOTIFY:
        returnVal = ctrl->notify((int)wParam, lParam);                        // controllerID, lParam
        break;

    case WM_CONTEXTMENU:
        returnVal = ctrl->contextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));    // handle, x, y (from screen coords)

    //case WM_ERASEBKGND:
    //    returnVal = ctrl->eraseBkgnd((HDC)wParam);                            // handle to device context
    //    break;

    default:
        returnVal = ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	if(!ctrl)
		return ::DefWindowProc(hwnd, msg, wParam, lParam);

	return returnVal;
}

LRESULT CALLBACK Win::GraphWinproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT returnVal = 0;

	/* Find the control associates with this window handle */
	static Win::Controller *ctrl;
	
	if(msg == WM_NCCREATE) {
		/*
			Message WM_NCCREATE is called before non-client parts (borders, titlebar, menu..etc)
			are created. This message comes with a pointer to CREATESTRUCT in lParam. The lpCreateParams
			member of CREATESTRUCT actually contains value of the lParam of CreateWindowEx().
		*/
		ctrl = (Controller*)(((CREATESTRUCT*)lParam)->lpCreateParams);
		ctrl->setHandle(hwnd);

		/* After get Controller, we store it into GWLP_USERDATA */
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctrl);

		return ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	ctrl = (Controller*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
		case WM_CREATE:
			returnVal = ctrl->create();
			break;

		case WM_SIZE:
			//width, height, type
			returnVal = ctrl->size(LOWORD(lParam), HIWORD(lParam), (int)wParam);
			break;

		case WM_ENABLE:
			returnVal = ctrl->enable(wParam!=0);
			break;

		case WM_PAINT:
			ctrl->paint();
			returnVal = ::DefWindowProcA(hwnd, msg, wParam, lParam);
			break;

		case WM_COMMAND:
			returnVal = ctrl->command(LOWORD(wParam), HIWORD(wParam), lParam); //id, code, msg
			break;

		case WM_CLOSE:
			returnVal = ctrl->close();
			break;

		case WM_DESTROY:
			returnVal = ctrl->destroy();
			break;

		case WM_SYSCOMMAND:
			//returnValue = ctrl->sysCommand(wParam, lParam);
			returnVal = ::DefWindowProc(hwnd, msg, wParam, lParam);
		/*
				// Disable Alt-F4 and screensavers
				switch (wparam & 0xfff0)
				{
				case SC_CLOSE:
				case SC_SCREENSAVE:
					ret = 0;
					break;

				default:
					returnValue = DefWindowProc(hwnd, message, wparam, lparam);
					break;
				}
				break;
		*/
			break;

			case WM_CHAR:
        //returnValue = ctrl->char(wParam, lParam);   // route keycode
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        returnVal = ctrl->keyDown((int)wParam, lParam);                       // keyCode, keyDetail
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        returnVal = ctrl->keyUp((int)wParam, lParam);                         // keyCode, keyDetail
        break;

    case WM_LBUTTONDOWN:
        returnVal = ctrl->lButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        //returnValue = ctrl->lButtonDown(wParam, GET_X_LPARAM(lParam), GET_X_LPARAM(lParam)); // state, x, y
        break;

    case WM_LBUTTONUP:
        returnVal = ctrl->lButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_RBUTTONDOWN:
        returnVal = ctrl->rButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        break;

    case WM_RBUTTONUP:
        returnVal = ctrl->rButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_MBUTTONDOWN:
        returnVal = ctrl->mButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        break;

    case WM_MBUTTONUP:
        returnVal = ctrl->mButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_MOUSEHOVER:
        returnVal = ctrl->mouseHover((int)wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, x, y
        break;

    case WM_MOUSELEAVE:
        returnVal = ctrl->mouseLeave();
        break;

    case WM_MOUSEMOVE:
        returnVal = ctrl->mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));  // state, x, y
        break;

    case WM_MOUSEWHEEL:
        returnVal = ctrl->mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam)/WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, delta, x, y
        break;

    case WM_HSCROLL:
        returnVal = ctrl->hScroll(wParam, lParam);
        break;

    case WM_VSCROLL:
        returnVal = ctrl->vScroll(wParam, lParam);
        break;

    case WM_TIMER:
        returnVal = ctrl->timer(LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_NOTIFY:
        returnVal = ctrl->notify((int)wParam, lParam);                        // controllerID, lParam
        break;

    case WM_CONTEXTMENU:
        returnVal = ctrl->contextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));    // handle, x, y (from screen coords)

    case WM_ERASEBKGND:
        //returnVal = ctrl->eraseBkgnd((HDC)wParam);                            // handle to device context
        return 1;

    default:
        returnVal = ::DefWindowProc(hwnd, msg, wParam, lParam);
	}

	if(!ctrl)
		return ::DefWindowProc(hwnd, msg, wParam, lParam);

	return returnVal;
}

INT_PTR CALLBACK Win::dialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// find controller associated with window handle
    static Win::Controller *ctrl;
    ctrl = (Controller*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    // WM_INITDIALOG message is called before displaying the dialog box.
    // lParam contains the value of dwInitParam of CreateDialogBoxParam(),
    // which is the pointer to the Controller.
    if(msg == WM_INITDIALOG) {
        // First, retrieve the pointrer to the controller associated with
        // the current dialog box.
        ctrl = (Controller*)lParam;
        ctrl->setHandle(hwnd);

        // Second, store the pointer to the Controller into GWL_USERDATA,
        // so, other messege can be routed to the associated Controller.
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctrl);

        // When WM_INITDIALOG is called, all controls in the dialog are created.
        // It is good time to initalize the appearance of controls here.
        // NOTE that we don't handle WM_CREATE message for dialogs because
        // the message is sent before controls have been create, so you cannot
        // access them in WM_CREATE message handler.
        ctrl->create();

        return true;
    }

    // check NULL pointer, because GWL_USERDATA is initially 0, and
    // we store a valid pointer value when WM_NCCREATE is called.
    if(!ctrl)
        return false;

    switch(msg)
    {
    case WM_COMMAND:
        ctrl->command(LOWORD(wParam), HIWORD(wParam), lParam);   // id, code, msg
        return true;

    case WM_TIMER:
        ctrl->timer(LOWORD(wParam), HIWORD(wParam));
        return true;

    case WM_PAINT:
        ctrl->paint();
        ::DefWindowProc(hwnd, msg, wParam, lParam);
        return true;

    case WM_DESTROY:
        ctrl->destroy();
        return true;

    case WM_CLOSE:
        ctrl->close();
        return true;

    case WM_HSCROLL:
        ctrl->hScroll(wParam, lParam);
        return true;

    case WM_VSCROLL:
        ctrl->vScroll(wParam, lParam);
        return true;

    case WM_NOTIFY:
        ctrl->notify((int)wParam, lParam);                      // controllerID, lParam
        return true;

    case WM_MOUSEMOVE:
        ctrl->mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
        //ctrl->mouseMove(wParam, (int)GET_X_LPARAM(lParam), (int)GET_Y_LPARAM(lParam));  // state, x, y
        return true;

    case WM_LBUTTONUP:
        ctrl->lButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));    // state, x, y
        return true;

    case WM_CONTEXTMENU:
        ctrl->contextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));    // handle, x, y (from screen coords)
        return true;

    default:
        return false;
    }
}
  