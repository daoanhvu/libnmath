#ifndef _CONTROLLERMAIN_H
#define _CONTROLLERMAIN_H

#include <windows.h>
#include "controller.h"

namespace Win {
	class ControllerMain: public Controller {
		private:
			HWND glHandle;
			HWND consolePaneHandle;
		public:
			ControllerMain();
			~ControllerMain() 	{};
			
			int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
			int create();                               // for WM_CRERATE
			int close();
			int destroy();
			int size(int w, int h, WPARAM wParam);      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)
			
			void setGLHandle(HWND handle)		{ glHandle = handle; };
			void setConsoleHandle(HWND handle)		{ consolePaneHandle = handle; };
	};
}

#endif