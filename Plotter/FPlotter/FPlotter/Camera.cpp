#include "Camera.h"

#include <glm\gtc\matrix_transform.hpp>

Camera::Camera(void) {
	mvp = glm::mat4();
	perspective = glm::mat4();
	model = glm::mat4();
	view = glm::mat4();
}

Camera::~Camera(void) {
}

void Camera::setup(float ex, float ey, float ez,
			float cx, float cy, float cz, float ux, float uy, float uz) {
	view = glm::lookAt(glm::vec3(ex,ey,ez), glm::vec3(cx,cy,cz),glm::vec3(ux,uy,uz));
}

void Camera::setPerspective(float fov, float near, float far) {
	float aspect = (viewport[2] - viewport[0])/(viewport[3] - viewport[1]);
	perspective = glm::perspective(fov, aspect, near, far);
}

void Camera::setOrtho(float left, float top, float right, float bottom) {
	orthor = glm::ortho(left, right, bottom, top);
}

void Camera::setViewport(int left, int top, int right, int bottom) {
	viewport = glm::vec4(left, top, right, bottom);
}

/*
	@param inPoint 3D point in world
	@param out 3D point on screen
*/
void Camera::world2Screen(float *out, const float *inPoint, char pm) {
	glm::vec3 result;

	if(pm == 'O') {
		result = glm::project(glm::vec3(inPoint[0], inPoint[1], inPoint[2]), view * model, orthor, viewport);
	} else {
		result = glm::project(glm::vec3(inPoint[0], inPoint[1], inPoint[2]), view * model, perspective, viewport);
	}

	out[0] = result.x;
	out[1] = result.y;
	out[2] = result.z;
}
