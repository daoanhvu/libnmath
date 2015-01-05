#ifndef _NEWFUNCTIONDLGCTL_H
#define _NEWFUNCTIONDLGCTL_H

#include "controller.h"

namespace Win {
	class NewFunctionDlgCtrl: public Controller {
	private:

	public:
		NewFunctionDlgCtrl(void);
		~NewFunctionDlgCtrl(void);

		int close();
	};
}

#endif