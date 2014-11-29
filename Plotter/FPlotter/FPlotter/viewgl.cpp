#include "viewgl.h"
#include "resource.h"

using namespace Win;

ViewGL::ViewGL(void): mHdc(0) {
}


ViewGL::~ViewGL(void) {
}

bool ViewGL::setPixelFormat(HDC hdc, int colorBits, int depthBits, int tencilBits) {
	PIXELFORMATDESCRIPTOR pfd;
	int pixelFormat = findPixelFormat(hdc, colorBits, depthBits, tencilBits);
	if(pixelFormat == 0)
		return false;

	::DescribePixelFormat(hdc, pixelFormat, sizeof(pfd), &pfd);

	if(!::SetPixelFormat(hdc, pixelFormat, &pfd)) {
		return false;
	}

	return true;
}

int ViewGL::findPixelFormat(HDC hdc, int colorBits, int depthBits, int stencilBits) {
	int currMode = 0;
	int bestMode = 0;
	int currScore = 0;
	int bestScore = 0;
	PIXELFORMATDESCRIPTOR pfd;

	for(currMode=1; ::DescribePixelFormat(hdc, currMode, sizeof(pfd), &pfd) > 0; currMode++) {
		//ignore if cannot support opengl or cannot render into a window
		if(!(pfd.dwFlags & PFD_SUPPORT_OPENGL) || !(pfd.dwFlags & PFD_DRAW_TO_WINDOW))
			continue;

		// ignore if cannot support rgba mode
        if((pfd.iPixelType != PFD_TYPE_RGBA) || (pfd.dwFlags & PFD_NEED_PALETTE))
            continue;

        // ignore if not double buffer
        if(!(pfd.dwFlags & PFD_DOUBLEBUFFER))
            continue;

		// colour bits
        if(pfd.cColorBits >= colorBits) ++currScore;

        // depth bits
        if(pfd.cDepthBits >= depthBits) ++currScore;

        // stencil bits
        if(pfd.cStencilBits >= stencilBits) ++currScore;

        // alpha bits
        if(pfd.cAlphaBits > 0) ++currScore;

        // check if it is best mode so far
        if(currScore > bestScore) {
            bestScore = currScore;
            bestMode = currMode;
        }
	}

	return bestMode;
}

void ViewGL::updateBuffer(int h, int w) {
}