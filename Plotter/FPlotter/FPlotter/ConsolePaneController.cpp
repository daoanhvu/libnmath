#include <Windows.h>
#include <CommCtrl.h>
#include "ConsolePaneController.h"
#include "resource.h"
#include "modelgl.h"

using namespace Win;


ConsolePaneController::ConsolePaneController(ModelGL *model, ConsoleView *view): mModel(model), mView(view) {
}


ConsolePaneController::~ConsolePaneController(void) {
}

int ConsolePaneController::close(){
	return 0;
}

//process WM_COMMAND
int ConsolePaneController::command(int id, int command, LPARAM msg) {
	static bool flag = false;

    switch(id) {
    case IDC_ANIMATE:
        if(command == BN_CLICKED) {
            flag = !flag;
            mModel->animate(flag);
            mView->animate(flag);
        }
        break;

    case IDC_FILL:
        if(command == BN_CLICKED) {
            mModel->setDrawMode(0);
        }
        break;

    case IDC_WIREFRAME:
        if(command == BN_CLICKED) {
            mModel->setDrawMode(1);
        }
        break;

    case IDC_POINTS:
        if(command == BN_CLICKED) {
            mModel->setDrawMode(2);
        }
        break;
    }

    return 0;
}

// for WM_CREATE
int ConsolePaneController::create() {
	// initialize all controls
    mView->initControls(mHandle);

    // place the opengl form dialog in right place, bottome of the opengl rendering window
    //RECT rect = {0, 0, 4, 8};
    //::MapDialogRect(glDialog.getHandle(), &rect);
    //int width = MulDiv(300, 4, rect.right);
    //::SetWindowPos(handle, HWND_TOP, 0, 300, 300, 200, 0);
	return 0;
}

// for WM_HSCROLL
int ConsolePaneController::hScroll(WPARAM wParam, LPARAM lParam) {
	// check if the message comming from trackbar
    HWND trackbarHandle = (HWND)lParam;

    int position = HIWORD(wParam);              // current tick mark position
    if(trackbarHandle) {
        // get control ID
        int trackbarId = ::GetDlgCtrlID(trackbarHandle);

        switch(LOWORD(wParam)){
        case TB_THUMBTRACK:     // user dragged the slider
            mView->updateTrackbars(trackbarHandle, position);
			/*
            if(trackbarId == IDC_RED)
                mModel->setBackgroundRed(position / 255.0f);
            else if(trackbarId == IDC_GREEN)
                mModel->setBackgroundGreen(position / 255.0f);
            else if(trackbarId == IDC_BLUE)
                mModel->setBackgroundBlue(position / 255.0f);
			*/
            break;

        case TB_THUMBPOSITION:  // by WM_LBUTTONUP
            break;

        case TB_LINEUP:         // by VK_RIGHT, VK_DOWN
            break;

        case TB_LINEDOWN:       // by VK_LEFT, VK_UP
            break;

        case TB_TOP:            // by VK_HOME
            break;

        case TB_BOTTOM:         // by VK_END
            break;

        case TB_PAGEUP:         // by VK_PRIOR (User click the channel to prior.)
            break;

        case TB_PAGEDOWN:       // by VK_NEXT (User click the channel to next.)
            break;

        case TB_ENDTRACK:       // by WM_KEYUP (User release a key.)
            position = (int)::SendMessage(trackbarHandle, TBM_GETPOS, 0, 0);
            mView->updateTrackbars(trackbarHandle, position);
			/*
            if(trackbarId == IDC_RED)
                mModel->setBackgroundRed(position / 255.0f);
            else if(trackbarId == IDC_GREEN)
                mModel->setBackgroundGreen(position / 255.0f);
            else if(trackbarId == IDC_BLUE)
                mModel->setBackgroundBlue(position / 255.0f);
			*/
            break;
        }
    }

    return 0;
}
