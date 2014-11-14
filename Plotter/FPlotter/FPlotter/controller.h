#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <windows.h>

namespace Win {
	class Controller {
		protected:
			HWND mHandle;
		public:
			Controller();
			virtual ~Controller();

			void setHandle(HWND handle);

			virtual int close();
			virtual int command(int id, int cmd, LPARAM msg);
			virtual int contextMenu(HWND, int x, int y);
			virtual int create();
			virtual int destroy();
			virtual int enable(bool flag);
			virtual int eraseBackground(HDC hdc);
			virtual int hScroll(WPARAM wparam, LPARAM lparam);
			virtual int vScroll(WPARAM wparam, LPARAM lparam);
			virtual int keyUp(int key, LPARAM lparam);
			virtual int keyDown(int key, LPARAM lparam);
			virtual int lButtonUp(WPARAM state, int x, int y);
			virtual int lButtonDown(WPARAM state, int x, int y);
			virtual int mButtonUp(WPARAM state, int x, int y);
			virtual int mButtonDown(WPARAM state, int x, int y);
			virtual int rButtonUp(WPARAM state, int x, int y);
			virtual int rButtonDown(WPARAM state, int x, int y);
			virtual int mouseHover(int state, int x, int y);
			virtual int mouseLeave();
			virtual int mouseMove(WPARAM state, int x, int y);
			virtual int mouseWheel(int state, int d, int x, int y);
			virtual int notify(int id, LPARAM lparam);
			virtual int paint();
			virtual int size(int w, int h, WPARAM wparam);
			virtual int timer(WPARAM id, LPARAM lparam);
	};

	inline void Controller::setHandle(HWND handle) { mHandle = handle; }
	inline int Controller::close() {::DestroyWindow(mHandle); return 0;}
}

#endif