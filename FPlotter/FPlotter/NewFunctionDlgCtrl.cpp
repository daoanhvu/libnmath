#include "NewFunctionDlgCtrl.h"

using namespace Win;

NewFunctionDlgCtrl::NewFunctionDlgCtrl(void)
{
}


NewFunctionDlgCtrl::~NewFunctionDlgCtrl(void)
{
}

int NewFunctionDlgCtrl::close() {
	//Close all child windows first
	//::EnumChildWindows(mHandle, (WNDENUMPROC)enumerateChildren, (LPARAM)WM_CLOSE);
	::DestroyWindow(mHandle);
	return 0;
}