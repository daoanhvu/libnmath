#ifndef _GLCONTROLLER_H
#define _GLCONTROLLER_H

#include "controller.h"
#include "viewgl.h"
#include "modelgl.h"

namespace Win {
	class GLController: public Controller {
		private:
			ModelGL *mModel;
			ViewGL *mView;
			HANDLE mThreadHandle;
			unsigned int mThreadId;
			volatile bool mLoopFlag;

			static void threadFunction(void *arg);
			void runThread();

		public:
			GLController(ModelGL *model, ViewGL *view);
			~GLController(void) {}
			int init();
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
	};
}
#endif