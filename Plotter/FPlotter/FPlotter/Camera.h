#ifndef _CAMERA_H
#define _CAMERA_H

#include <glm\glm.hpp>

class Camera {
	private:
		mat4 mvp;
		mat4 view;
		mat4 model;
		mat4 modelView;
		mat4 perspective;
		mat4 orthor;

	public:
		Camera(void);
		~Camera(void);

		void setup(float eyeX, float eyeY, float eyeZ, float targetX, float targetY, float targetZ);

		void world2Screen(float *out, float *inPoint);
};

#endif