#ifndef _VIEWGL_H
#define _VIEWGL_H

#include <Windows.h>

namespace Win {
	class ViewGL {
		private:
			HDC mHdc;				//handle to device context
			HBITMAP hBitmapBuffer;
			HDC hdcBuffer;

			static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);
			static int findPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);

		public:
			ViewGL(void);
			~ViewGL(void);

			HDC getDC() const { return mHdc; };

			void updateBuffer(int h, int w);
	};
}

#endif

