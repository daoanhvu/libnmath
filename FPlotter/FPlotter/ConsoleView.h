#ifndef _CONSOLEVIEW_H
#define _CONSOLEVIEW_H

#include <Windows.h>
#include "Controls.h"

namespace Win {
	class ConsoleView {
		private:
			// controls
			Win::Button buttonAnimate;
			Win::RadioButton radioFill;
			Win::RadioButton radioWireframe;
			Win::RadioButton radioPoint;
			Win::Trackbar trackbarRed;
			Win::Trackbar trackbarGreen;
			Win::Trackbar trackbarBlue;

		public:
			ConsoleView(void);
			~ConsoleView(void);

			void initControls(HWND handle);         // init all controls

			// update controls on the form
			void animate(bool flag);
			void updateTrackbars(HWND handle, int position);
	};
}

#endif