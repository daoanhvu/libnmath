#include "controller.h"

using namespace Win;

Controller::Controller():mHandle(0) {
}


Controller::~Controller() {
	::DestroyWindow(mHandle);
}
