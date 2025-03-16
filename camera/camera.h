#ifndef _CAMERA_H
#define _CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fp {
	class Camera {
	private:
		glm::mat4 view;
		glm::mat4 mModel;
		glm::mat4 perspective;
		glm::mat4 orthor;
		float viewport[4];

		glm::vec3 center;

		// This is in radian
		float fov;

		float eyeX;
		float eyeY;
		float eyeZ;

		glm::vec3 up;

		float pitchAccum;
		float yawAccum;
		float rollAccum;

		glm::mat4 inverted;
		glm::mat4 rotation;
		glm::mat4 temp;
		glm::vec4 xAxis, yAxis;
		glm::vec3 rotationAxisX;
		glm::vec3 rotationAxisY;
		float rotX;
		float rotY;

	public:
		Camera();
		virtual ~Camera() {}

		void initialize();

		//void setModelMatrix(float *mm);
		//void setViewMatrix(float *vm);

		void lookAt(float ex, float ey, float ez,
					float cx, float cy, float cz,
					float ux, float uy, float uz);
		void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
		void setPerspective(float fov, float nearPlane, float farPlane);
		void setOrtho(float left, float top, float right, float bottom);
		void rotate(float yaw, float pitch, float roll);
		void setViewport(int left, int top, int right, int bottom);
		void project(float *out, const float *obj);
		void project(float *out, float objX, float objY, float objZ);
		void projectOrthor(float *out, const float *obj);
		void projectOrthor(float *out, float objX, float objY, float objZ);

		//on testing method
		void moveAlongForward(float d);
		
		void getView(float *viewOut);
		void getPerspective(float *pOut);
		glm::mat4 getMVP(glm::mat4& modelView);
	};
}

#endif
