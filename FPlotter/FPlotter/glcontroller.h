#ifndef _GLCONTROLLER_H
#define _GLCONTROLLER_H

#include "controller.h"
#include "modelgl.h"

namespace Win {
	class GLController: public Controller {
		private:

			ModelGL *mModel;

			HANDLE mThreadHandle;
			unsigned int mThreadId;
			volatile bool mLoopFlag;

			//Memervariables use for viewing
			HDC mHdc;				//handle to device context
			HBITMAP hMemoryBitmap;
			HDC memoryDC;
			int mWidth;
			int mHeight;
			PAINTSTRUCT mPS;
			RECT mClientRect;
			int mMouseX;
			int mMouseY;

			static void threadFunction(void *arg);
			void runThread();

		public:
			GLController(ModelGL *model);
			~GLController(void) {}

			int close();
			int command(int id, int cmd, LPARAM msg);
			int create();
			int paint();
			int size(int w, int h, WPARAM wParam);
			int lButtonDown(WPARAM state, int x, int y);
			int lButtonUp(WPARAM state, int x, int y);
			int rButtonDown(WPARAM state, int x, int y);
			int rButtonUp(WPARAM state, int x, int y);
			int mouseMove(WPARAM state, int x, int y);
			int onOtherMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

			//Methods for Viewing
			void releaseBuffer();
			HDC getDC() const { return mHdc; };
			void updateBuffer(int w, int h);
			void invalidate(const RECT* rect, BOOL erase);
	};

	float angle2DVector(float x1, float y1, float x2, float y2);
	glm::vec4 quaternion(glm::vec3 va, float phi);
}
#endif