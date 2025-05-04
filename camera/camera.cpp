#include "camera.h"
#include <cmath>
#include <fstream>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifdef _ADEBUG
//#include <stdio.h>
//#include <iostream>
#include <android/log.h>
#define LOG_TAG "NativeCamera"
#define LOG_LEVEL 10
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

namespace fp {

	Camera::Camera() : pitchAccum(0), yawAccum(0), rollAccum(0), mModel(glm::mat4(1.0f)) {
		xAxis = glm::vec4(1, 0, 0, 0);
		yAxis = glm::vec4(0, 1, 0, 0);
		this->mModel = glm::mat4(1.0f);
		this->rotation = glm::mat4(1.0f);
	}

	void Camera::lookAt(float ex, float ey, float ez,
						float cx, float cy, float cz, float ux, float uy, float uz) {
		up = glm::vec3(ux, uy, uz);
		glm::vec3 eye(ex, ey, ez);
		center = glm::vec3(cx, cy, cz);

		eyeX = ex;
		eyeY = ey;
		eyeZ = ez;
		view = glm::lookAt(eye, center, up);
	}

	void Camera::setPerspective(float fov, float nearPlane, float farPlane) {
		float aspect = (viewport[2] - viewport[0]) / (viewport[3] - viewport[1]);
		perspective = glm::perspective(fov, aspect, nearPlane, farPlane);
	}

	void Camera::setOrtho(float left, float top, float right, float bottom) {
		orthor = glm::ortho(left, right, bottom, top);
	}

	void Camera::setViewport(int left, int top, int right, int bottom) {
		viewport[0] = left;
		viewport[1] = top;
		viewport[2] = right;
		viewport[3] = bottom;
	}

/*
	yaw: rotate around right vector
	pitch: rotate around up vector
*/
	void Camera::rotate(float yawR, float pitchR, float roll) {
//	std::ofstream f("storage/sdcard0/data_rotation.txt", std::ofstream::app);
		rotX = -1 * pitchR * 0.05f;
		rotY = -1 * yawR * 0.05f;

//	f << "\n New Rotation dx = " << yawR << ", dy = " << pitchR << "\n";
//	f << "model matrix: \n";
//	f << mModel;
		inverted = glm::inverse(mModel);
//	f << "\n inverted model matrix 1: \n";
//	f << inverted;
		rotationAxisX = inverted * xAxis;
//	f << "\n RotationAsixX: \n";
//	f << rotationAxisX;
		mModel = glm::rotate(mModel, rotX, rotationAxisX);
//	rotation = gm::rotate(rotX, rotationAxisX);
//	f << "\n Rotation matrix about RotationAxisX: \n";
//	f << rotation;
//	temp = mModel * rotation;
//	f << "\n Model matrix after rotate by RotationAxisX: \n";
//	f << temp;

		inverted = glm::inverse(mModel);
		//temp.inverse2(inverted);
//	f << "\n inverted model matrix 2: \n";
//	f << inverted;
		rotationAxisY = inverted * yAxis;
//	f << "\n RotationAsixY: \n";
//	f << rotationAxisY;
		mModel = glm::rotate(mModel, rotY, rotationAxisY);
//	rotation = gm::rotate(rotY, rotationAxisY);
//	f << "\n Rotation matrix about RotationAxisY: \n";
//	f << rotation;
//	mModel = temp * rotation;
//	f << "\n Final Model matrix : \n";
//	f << mModel << "\n\n";

//	f.close();
	}

/*
	@param inPoint 3D point in world
	@param out 2D point on screen
*/
	void Camera::project(float *out, const float *obj) {
		glm::vec4 tmp(obj[0], obj[1], obj[2], 1);
		tmp = (view * mModel) * tmp;
		tmp = perspective * tmp;
		tmp /= tmp[3]; //tmp.w

		tmp = tmp * 0.5f + 0.5f;
		out[0] = tmp[0] * viewport[2] + viewport[0];
		out[1] = tmp[1] * viewport[3] + viewport[1];
		out[2] = tmp[2];
	}

/**
 * @param out output parameter, must be 3 in length array (or more)
 */
	void Camera::project(float *out, float objX, float objY, float objZ) {
		float result;
		glm::vec4 tmp(objX, objY, objZ, 1);
		tmp = (view * mModel) * tmp;
		tmp = perspective * tmp;
		tmp /= tmp[3]; //tmp.w

		tmp = tmp * 0.5f + 0.5f;
		out[0] = tmp[0] * viewport[2] + viewport[0];
		out[1] = tmp[1] * viewport[3] + viewport[1];
		out[2] = tmp[2];
	}

	void Camera::projectOrthor(float *out, const float *obj) {
		glm::vec4 tmp(obj[0], obj[1], obj[2], 1);
		tmp = (view * mModel) * tmp;
		tmp = orthor * tmp;
		tmp /= tmp[3];

		//Should use memcpy
		tmp = tmp * 0.5f + 0.5f;
		out[0] = tmp[0] * viewport[2] + viewport[0];
		out[1] = tmp[1] * viewport[3] + viewport[1];
		out[2] = tmp[2];
	}

	void Camera::projectOrthor(float *out, float objX, float objY, float objZ) {
		float result;
		glm::vec4 tmp(objX, objY, objZ, 1);
		tmp = (view * mModel) * tmp;
		tmp = orthor * tmp;
		tmp /= tmp[3]; //tmp.w

		tmp = tmp * 0.5f + 0.5f;
		out[0] = tmp[0] * viewport[2] + viewport[0];
		out[1] = tmp[1] * viewport[3] + viewport[1];
		out[2] = tmp[2];
	}

/**
 * on testing method
 */
	void Camera::moveAlongForward(float d) {
	}

  void Camera::getView(float *viewOut) {
    const float *viewPtr = glm::value_ptr(this->view);
    std::copy(viewPtr, viewPtr + 16, viewOut);
  }

  void Camera::getPerspective(float *pOut) {
    const float *pPtr = glm::value_ptr(this->perspective);
    std::copy(pPtr, pPtr + 16, pOut);
  }

	glm::mat4 Camera::getMVP(glm::mat4 &modelView) {
		modelView = this->view * this->mModel;
		return (this->perspective * modelView);
	}
}