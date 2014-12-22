#ifndef _MODELGL_H
#define _MODELGL_H

#include <Windows.h>
#include <vector>
#include <nmath.h>
#include "Camera.h"

using namespace nmath;

class ModelGL {

	typedef struct tIndex {
		int *indice;
		int size;
	} TIndex;
	
	private:
		void initLights();
		void createObject();
		unsigned int loadTextureBmp(const char *filename);

		// members
		int windowWidth;
		int windowHeight;
		RECT mClientRect;
		bool windowResized;
		Camera camera;

		bool animateFlag;                               // on/off animation
		bool changeDrawMode;
		int drawMode;
		unsigned int listId;                            // display list ID
		bool mouseLeftDown;
		bool mouseRightDown;
		int mouseX;
		int mouseY;
		float cameraAngleX;
		float cameraAngleY;
		float cameraDistance;
		BYTE bgColor[4];
		bool bgFlag;
		unsigned char* frameBuffer;                     // framebuffer to store RGBA color
		int bufferSize;                                 // framebuffer size in bytes

		NFunction *mFunction;
		
		ListFData *mVertexData;
		TIndex *mIndice;
		int numOfIndice;

		int mStatus;
		
		HBRUSH mBackgroundBrush;

	protected:

	public:
		ModelGL();
		~ModelGL();

		void init();                                    // initialize OpenGL states
		void release();
		void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
		void setViewport(int width, int height);
		void resizeWindow(int width, int height);
		void draw(HDC hdc);

		void setMouseLeft(bool flag) { mouseLeftDown = flag; };
		void setMouseRight(bool flag) { mouseRightDown = flag; };
		void setMousePosition(int x, int y) { mouseX = x; mouseY = y; };
		void setDrawMode(int mode);
		void animate(bool flag) { animateFlag = flag; };

		void onMouseMove();
		void rotateCamera(float beta);
		void zoomCamera(int dist);

		void setRotateMode(int mode) { mStatus = mode; }
		bool isRotating() { return mStatus;}

		void setBackgroundColor(BYTE red, BYTE green, BYTE, BYTE blue);             // change background colour
};

#endif