#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <Windows.h>

namespace Win {
	class Controller {
		protected:
			HWND mHandle;
		public:
			Controller();
			virtual ~Controller(void);

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

			virtual int onOtherMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

	inline void Controller::setHandle(HWND handle) { mHandle = handle; }
	inline int Controller::close() { ::DestroyWindow(mHandle); return 0; }
	inline int Controller::command(int id, int cmd, LPARAM msg) { return 0; }
    inline int Controller::contextMenu(HWND handle, int x, int y) { return 0; }
    inline int Controller::create() { return 0; }
    inline int Controller::destroy() { return 0; }
    inline int Controller::enable(bool flag) { return 0; }
    inline int Controller::eraseBackground(HDC hdc) { return 0; }
    inline int Controller::hScroll(WPARAM wParam, LPARAM lParam) { return 0; }
    inline int Controller::keyDown(int key, LPARAM lParam) { return 0; }
    inline int Controller::keyUp(int key, LPARAM lParam) { return 0; }
    inline int Controller::lButtonDown(WPARAM wParam, int x, int y) { return 0; }
    inline int Controller::lButtonUp(WPARAM wParam, int x, int y) { return 0; }
    inline int Controller::mButtonDown(WPARAM wParam, int x, int y) { return 0; }
    inline int Controller::mButtonUp(WPARAM wParam, int x, int y) { return 0; }
    inline int Controller::mouseHover(int state, int x, int y) { return 0; }
    inline int Controller::mouseLeave() { return 0; }
    inline int Controller::mouseMove(WPARAM keyState, int x, int y) { return 0; }
    inline int Controller::mouseWheel(int state, int delta, int x, int y) { return 0; }
    inline int Controller::notify(int id, LPARAM lParam) { return 0; }
    inline int Controller::paint() { return 0; }
    inline int Controller::rButtonDown(WPARAM wParam, int x, int y) { return 0; }
    inline int Controller::rButtonUp(WPARAM wParam, int x, int y) { return 0; }
    inline int Controller::size(int w, int h, WPARAM type) { return 0; }
    inline int Controller::timer(WPARAM id, LPARAM lParam) { return 0; }
    inline int Controller::vScroll(WPARAM wParam, LPARAM lParam) { return 0; }
	inline int Controller::onOtherMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { return 0; }

}

#endif