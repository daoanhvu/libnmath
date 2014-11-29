#ifndef _CONSOLEPANECONTROLLER_H
#define _CONSOLEPANECONTROLLER_H

#include "controller.h"
#include "modelgl.h"
#include "ConsoleView.h"

namespace Win {
	class ConsolePaneController: public Controller	{
		private:
			ModelGL *mModel;
			ConsoleView *mView;

		public:
			ConsolePaneController(ModelGL *model, ConsoleView *view);
			~ConsolePaneController(void);

			int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
			int create();                               // for WM_CREATE
			int close();
			int hScroll(WPARAM wParam, LPARAM lParam);  // for WM_HSCROLL
	};

}
#endif