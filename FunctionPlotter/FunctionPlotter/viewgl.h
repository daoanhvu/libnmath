#ifndef _VIEWGL_H
#define _VIEWGL_H

#include <Windows.h>
#include <GL\glew.h>
#include <GL\wglew.h>
#include <GL\GL.h>
#include <GL\glu.h>

namespace Win {
	class ViewGL
	{
		private:
			HDC mHdc;		//handle to device context
			HGLRC mHglrc;	//handle to OpenGL rendering context
			GLuint mProgramID;
			GLuint mPositionID;
			GLuint mEnableLightID;
			int windowWidth;
			int windowHeight;
			bool windowResized;

			static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);
			static int findPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits);
			static GLuint loadShader(const char *vertexShaderFile, const char *fragmentShaderFile);

		public:
			ViewGL(void);
			~ViewGL(void);

			int init();
			void setViewport(int width, int height);
			void resizeWindow(int width, int height);
			bool createContext(HWND handle, int colorBits, int depthBits, int stencilBits);  // create OpenGL rendering context
			void closeContext(HWND handle);
			void swapBuffers();

			bool resetViewportIfNeeded();

			HDC getDC() const { return mHdc; };
			HGLRC getRC() const { return mHglrc; };
	};
}

#endif

