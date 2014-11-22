#ifndef _VIEWGL_H
#define _VIEWGL_H

#include <Windows.h>

namespace Win {
	class ViewGL
	{
		private:
			HDC mHdc;		//handle to device context
			HGLRC mHglrc;	//handle to OpenGL rendering context

			static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);
			static int findPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);

		public:
			ViewGL(void);
			~ViewGL(void);

			bool createContext(HWND handle, int colorBits, int depthBits, int stencilBits);  // create OpenGL rendering context
			void closeContext(HWND handle);
			void swapBuffers();

			HDC getDC() const { return mHdc; };
			HGLRC getRC() const { return mHglrc; };
	};
}

#endif

