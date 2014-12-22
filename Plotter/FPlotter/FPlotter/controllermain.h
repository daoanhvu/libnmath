#ifndef _CONTROLLERMAIN_H
#define _CONTROLLERMAIN_H

#include "controller.h"
#include "window.h"
#include "dialogwindow.h"

namespace Win {
	class ControllerMain: public Controller {
		private:
			Window *mWindow;

			//This is handle for drawing window
			HWND mDrawingHandle;

			//this is handle for control panel
			HWND consolePaneHandle;

			DialogWindow *dialog;
			bool mBttRotateStatus;
		public:
			ControllerMain();
			~ControllerMain();
			
			int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
			int create();                               // for WM_CRERATE
			int close();
			int destroy();
			int size(int w, int h, WPARAM wParam);      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)
			
			void setDrawingHandle(HWND handle)	{ mDrawingHandle = handle; }
			void setConsoleHandle(HWND handle)	{ consolePaneHandle = handle; }
			void setWindow(Window* window)		{ mWindow = window; }
	};
}

#endif