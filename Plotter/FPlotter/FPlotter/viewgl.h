#ifndef _VIEWGL_H
#define _VIEWGL_H

#include <Windows.h>
#include "modelgl.h"

namespace Win {
	class ViewGL {
		private:
			HDC mHdc;				//handle to device context
			HBITMAP hMemoryBitmap;
			HDC memoryDC;
			int mWidth;
			int mHeight;
			PAINTSTRUCT mPS;
			RECT mClientRect;

			//static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);

		public:
			ViewGL(void);
			~ViewGL(void);

			
	};
}

#endif

