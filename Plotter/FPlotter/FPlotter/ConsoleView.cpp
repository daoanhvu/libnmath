#include "ConsoleView.h"
#include "resource.h"

using namespace Win;

ConsoleView::ConsoleView(void) {
}


ConsoleView::~ConsoleView(void) {
}

// init all controls
void ConsoleView::initControls(HWND handle) {
	// set all controls
    buttonAnimate.set(handle, IDC_ANIMATE);
    radioFill.set(handle, IDC_FILL);
    radioWireframe.set(handle, IDC_WIREFRAME);
    radioPoint.set(handle, IDC_POINTS);
    trackbarRed.set(handle, IDC_RED);
    trackbarGreen.set(handle, IDC_GREEN);
    trackbarBlue.set(handle, IDC_BLUE);

    // initial state
    radioFill.check();
    trackbarRed.setRange(0, 255);
    trackbarRed.setPos(0);
    trackbarGreen.setRange(0, 255);
    trackbarGreen.setPos(0);
    trackbarBlue.setRange(0, 255);
    trackbarBlue.setPos(0);
}


// update controls on the form
void ConsoleView::animate(bool flag) {
	if(flag)
        buttonAnimate.setText(L"Stop");
    else
        buttonAnimate.setText(L"Animate");
}

void ConsoleView::updateTrackbars(HWND handle, int position) {
    if(handle == trackbarRed.getHandle()) {
        trackbarRed.setPos(position);
    } else if(handle == trackbarGreen.getHandle()) {
        trackbarGreen.setPos(position);
    } else if(handle == trackbarBlue.getHandle()) {
        trackbarBlue.setPos(position);
    }
}
