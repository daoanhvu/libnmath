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
			HBRUSH mPaintBrush;
			HBRUSH mBackgroundBrush;
			RECT mClientRect;
			ModelGL *mModel;

			//static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);

		public:
			ViewGL(void);
			~ViewGL(void);

			void releaseBuffer();
			void paint(HWND handle);
			HDC getDC() const { return mHdc; };
			void updateBuffer(HWND handle, int w, int h);
	};
}

#endif

