#ifndef _MODELGL_H
#define _MODELGL_H

#include <vector>
#include <nmath.h>
#include <criteria.h>
#include "Camera.h"

class ModelGL {
	private:
		void initLights();
		void createObject();
		unsigned int loadTextureBmp(const char *filename);

		// members
		int windowWidth;
		int windowHeight;
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
		bool windowResized;
		unsigned char* frameBuffer;                     // framebuffer to store RGBA color
		int bufferSize;                                 // framebuffer size in bytes

		Function *mFunction;
		ListFData *data; 

	protected:

	public:
		ModelGL();
		~ModelGL();

		void init();                                    // initialize OpenGL states
		void release();
		void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
		void setViewport(int width, int height);
		void resizeWindow(int width, int height);
		void draw();

		void setMouseLeft(bool flag) { mouseLeftDown = flag; };
		void setMouseRight(bool flag) { mouseRightDown = flag; };
		void setMousePosition(int x, int y) { mouseX = x; mouseY = y; };
		void setDrawMode(int mode);
		void animate(bool flag) { animateFlag = flag; };

		void rotateCamera(int x, int y);
		void zoomCamera(int dist);

		void setBackgroundColor(BYTE red, BYTE green, BYTE, BYTE blue);             // change background colour
};

#endif