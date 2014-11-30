#ifndef _CAMERA_H
#define _CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm\glm.hpp>

class Camera {
	private:
		glm::mat4 mvp;
		glm::mat4 view;
		glm::mat4 model;
		glm::mat4 modelView;
		glm::mat4 perspective;
		glm::mat4 orthor;
		glm::vec4 viewport;

	public:
		Camera(void);
		~Camera(void);

		void setup(float ex, float ey, float ez,
			float cx, float cy, float cz, float ux, float uy, float uz);
		void setPerspective(float fov, float near, float far);
		void setOrtho(float left, float top, float right, float bottom);

		void setViewport(int left, int top, int right, int bottom);
		void world2Screen(float *out, const float *inPoint, char projectionUsed);
};

#endif