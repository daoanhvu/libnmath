#ifndef DIALOG_WINDOW_H
#define DIALOG_WINDOW_H

#include <Windows.h>
#include "controller.h"

namespace Win {
	class DialogWindow	{
		private:
			HWND mHandle;
			HWND mParentHandle;
			WORD mId;
			int mX;
			int mY;
			int mWidth;
			int mHeight;
			HINSTANCE mInstance;
			Controller *mController;

		public:
			DialogWindow(HINSTANCE inst, WORD id, HWND parent, Controller *ctrl);
			~DialogWindow(void);

			HWND create();
			void show(int cmdShow=SW_SHOWDEFAULT);
			HWND getHandle()	{ return mHandle; }

			// setters
			void setPosition(int x, int y)          { this->mX = x; this->mY = y; }
			void setWidth(int w)                    { mWidth = w; }
			void setHeight(int h)                   { mHeight = h; }
			Controller* getController()			{ return mController; }
	};
}

#endif