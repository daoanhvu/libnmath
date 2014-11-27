#ifndef _VIEWGL_H
#define _VIEWGL_H

#include <Windows.h>
#include <GL\glew.h>
#include <GL\wglew.h>
#include <GL\GL.h>
#include <GL\glu.h>

namespace Win {
	class ViewGL {
		private:
			HWND mHandle;
			HWND mParentHandle;
			HINSTANCE mInstance;
			WNDCLASSEX winClass;
			DWORD winStyle;
			DWORD winStyleEx;
			HDC mHdc;			//handle to device context
			HGLRC mHglrc;		//handle to OpenGL rendering context
			GLuint mProgramID;
			GLuint mPositionID;
			GLuint mEnableLightID;
			int x;
			int y;
			int windowWidth;
			int windowHeight;
			bool windowResized;

			unsigned char mMajorVersion;
			unsigned char mMinorVersion;

			enum { MAX_STRING = 256 };              // local constants, max length of string
			wchar_t title[MAX_STRING];
			wchar_t className[MAX_STRING];

			int init();

			static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);
			static int findPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);
			static GLuint loadShader(const char *vertexShaderFile, const char *fragmentShaderFile);

		public:
			ViewGL(HINSTANCE hInstance, HWND parent, const wchar_t *name, int width, int height,
				DWORD clsStyle, DWORD wStyle, unsigned char majorv, unsigned char minorv);
			~ViewGL(void);

			void setViewport(int width, int height);
			void resizeWindow(int width, int height);
			HWND createWindow(void *controller);
			int createGLContext(int colorBits, int depthBits, int stencilBits);  // create OpenGL rendering context
			void closeContext();
			void swapBuffers();

			bool resetViewportIfNeeded();

			HDC getDC() const { return mHdc; };
			HGLRC getRC() const { return mHglrc; };

			void show(int shw = SW_SHOWDEFAULT);
			HWND getHandle()						{ return mHandle; }

			void setClassStyle(UINT style)			{ winClass.style = style; }
			void setBackgroundColor(int color)		{ winClass.hbrBackground = (HBRUSH)::GetStockObject(color); }
			void setIcon(int id);
			void setIconSmall(int id);
			void setCursor(int id);
			void setMenuName(LPCTSTR name)			{ winClass.lpszMenuName = name; }

			void setWindowStyle(DWORD style)		{ winStyle = style; }
			void setWindowStyleEx(DWORD style)		{ winStyleEx = style; }
			void setPosition(int x, int y)			{ this->x = x; this->y = y; }
			void setWidth(int w)					{ windowWidth = w; }
			void setHeight(int h)					{ windowHeight = h; }
			void setSize(int w, int h)				{ windowWidth = w; windowHeight = h; }
			void setParent(HWND hwnd)				{ mParentHandle = hwnd; }
	};
}

#endif

