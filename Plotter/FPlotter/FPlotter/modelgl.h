#ifndef _MODELGL_H
#define MODELGL_H

class ModelGL {
	private:
		void initLights();
		void createObject();

	protected:
	public:
		ModelGL();
		~ModelGL();

		void init();
		void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
		void setViewport(int width, int height);
		void resizeWindow(int width, int height);
		void draw();


};

#endif