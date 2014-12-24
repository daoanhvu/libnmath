#include "Camera.h"

#include <common.h>
#include <vector>
#include <glm\ext.hpp>
#include <glm\gtx\matrix_cross_product.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

using namespace glm;

Camera::Camera(void) {
	mvp = glm::mat4();
	perspective = glm::mat4();
	model = glm::mat4();
	view = glm::mat4();

	pitchAccum = 0;
	yawAccum = 0;
	rollAccum = 0;
}

Camera::~Camera(void) {
}

void Camera::setup(float ex, float ey, float ez,
			float cx, float cy, float cz, float ux, float uy, float uz) {
	view = glm::lookAt(glm::vec3(ex,ey,ez), glm::vec3(cx,cy,cz),glm::vec3(ux,uy,uz));
}

void Camera::setPerspective(float fov, float nearPlane, float farPlane) {
	float aspect = (viewport[2] - viewport[0])/(viewport[3] - viewport[1]);
	perspective = glm::perspective(fov, aspect, nearPlane, farPlane);
}

void Camera::setOrtho(float left, float top, float right, float bottom) {
	orthor = glm::ortho(left, right, bottom, top);
}

void Camera::setViewport(int left, int top, int right, int bottom) {
	viewport = glm::vec4(left, top, right, bottom);
}

/*
	yaw: rotate around right vector
	pitch: rotate around up vector
*/
void Camera::rotate(float yawR, float pitchR, float roll) {

	yawR *= 0.05f;
	pitchR *= 0.05f;
	roll *= 0.05f;

	pitchAccum += pitchR;
	yawAccum += yawR;
	rollAccum += roll;

	// Rotate the camera about the world Y axis
	// N.B. 'angleAxis' method takes angle in degrees (not in radians)
	glm::quat rotation = glm::angleAxis(pitchAccum, glm::vec3(1, 0, 0)) * glm::angleAxis(yawAccum, glm::vec3(0, 1, 0))
		* glm::angleAxis(rollAccum, glm::vec3(0, 0, 1));
	model = glm::toMat4(rotation);
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

void Camera::drawTriangleTrip(HDC paint, const float* vertices, const int* indices, int indexSize) {
	int i = 0;
	bool gotDegen = true;
	glm::vec3 tmp(0, 0, 0);
	glm::vec3 pw0, p, p1, p2;
	glm::vec3 pw1;
	glm::vec3 pw2;

	//pw0[3] = 1.0f;
	//pw1[3] = 1.0f;
	//pw2[3] = 1.0f;
	if (indexSize > 2){
		//draw the first triangle
		pw0[0] = vertices[indices[0] * 3];
		pw0[1] = vertices[indices[0] * 3 + 1];
		pw0[2] = vertices[indices[0] * 3 + 2];

		pw1[0] = vertices[indices[1] * 3];
		pw1[1] = vertices[indices[1] * 3 + 1];
		pw1[2] = vertices[indices[1] * 3 + 2];

		pw2[0] = vertices[indices[2] * 3];
		pw2[1] = vertices[indices[2] * 3 + 1];
		pw2[2] = vertices[indices[2] * 3 + 2];
		p = glm::project(pw0, view*model, perspective, viewport);
		p1 = glm::project(pw1, view*model, perspective, viewport);
		p2 = glm::project(pw2, view*model, perspective, viewport);
		/*
		world2Screen(p, pw0);
		world2Screen(p1, pw1);
		world2Screen(p2, pw2);
		*/
		MoveToEx(paint, p[0], p[1], NULL);
		LineTo(paint, p1[0], p1[1]);

		MoveToEx(paint, p[0], p[1], NULL);
		LineTo(paint, p2[0], p2[1]);

		MoveToEx(paint, p1[0], p1[1], NULL);
		LineTo(paint, p2[0], p2[1]);
		for (i = 1; i<indexSize - 2; i++){
			if (indices[i] == indices[i + 1] || indices[i + 1] == indices[i + 2])
				gotDegen = true;
			else{
				if (gotDegen) {
					//just draw like the first triangle
					pw0[0] = vertices[indices[i] * 3];
					pw0[1] = vertices[indices[i] * 3 + 1];
					pw0[2] = vertices[indices[i] * 3 + 2];

					pw1[0] = vertices[indices[i + 1] * 3];
					pw1[1] = vertices[indices[i + 1] * 3 + 1];
					pw1[2] = vertices[indices[i + 1] * 3 + 2];

					pw2[0] = vertices[indices[i + 2] * 3];
					pw2[1] = vertices[indices[i + 2] * 3 + 1];
					pw2[2] = vertices[indices[i + 2] * 3 + 2];
					p = glm::project(pw0, view*model, perspective, viewport);
					p1 = glm::project(pw1, view*model, perspective, viewport);
					p2 = glm::project(pw2, view*model, perspective, viewport);

					MoveToEx(paint, p[0], p[1], NULL);
					LineTo(paint, p1[0], p1[1]);

					MoveToEx(paint, p[0], p[1], NULL);
					LineTo(paint, p2[0], p2[1]);

					MoveToEx(paint, p1[0], p1[1], NULL);
					LineTo(paint, p2[0], p2[1]);

					gotDegen = false;
				}
				else {
					pw0[0] = vertices[indices[i + 2] * 3];
					pw0[1] = vertices[indices[i + 2] * 3 + 1];
					pw0[2] = vertices[indices[i + 2] * 3 + 2];

					tmp = glm::project(pw0, view*model, perspective, viewport);
					MoveToEx(paint, p1[0], p1[1], NULL);
					LineTo(paint, tmp[0], tmp[1]);

					MoveToEx(paint, p2[0], p2[1], NULL);
					LineTo(paint, tmp[0], tmp[1]);

					p[0] = p1[0];
					p[1] = p1[1];

					p1[0] = p2[0];
					p1[1] = p2[1];

					p2[0] = tmp[0];
					p2[1] = tmp[1];
				}
			}
		}
	}
}

/**
* Need to be tested with OpenGL
* @param vertices
* @param rows each element in this array contains the number of vertex on the corresponding row
* @return
*/
int* Camera::buildIndicesForTriangleStrip(int vcount, int* rows, int rowCount, int *outSize) {
	int i, j, num_of_vertices, nextCol, count, diff, k;

	std::vector<int> indices;

	count = 0;
	for (i = 0; i<rowCount - 1; i++){
		num_of_vertices = rows[i];
		nextCol = rows[i + 1];

		for (j = 0; j<num_of_vertices; j++){
			indices.push_back(count);
			if (i>0 && j == 0){ //first element of rows that not the first row
				indices.push_back(count);
			}

			if (j == num_of_vertices - 1){
				if (i < rowCount - 2){
					if ((count + num_of_vertices) < vcount && j<nextCol){
						//neu co 1 phan tu ngay ben duoi
						indices.push_back(count + num_of_vertices);
						indices.push_back(count);
					}
					indices.push_back(count);
				}
				else	if ((count + num_of_vertices) < vcount && j<nextCol){
					k = count + num_of_vertices;
					while (k < vcount){
						indices.push_back(k);
						k++;
						if (k < vcount)
							indices.push_back(k);
						k++;
						if (k<vcount)
							indices.push_back(count);
					}
				}
			}
			else{
				//neu ngay ben duoc co mot vertex nua
				if ((count + num_of_vertices) < vcount && j<nextCol)
					indices.push_back(count + num_of_vertices);
				else{ //neu khong thi add vertex cuoi cung cua dong duoi
					diff = j - nextCol + 1;
					indices.push_back(count + num_of_vertices - diff);
				}
			}
			count++;
		}
	}
	*outSize = indices.size();
	int* result = new int[indices.size()];
	for (i = 0; i<(*outSize); i++){
		result[i] = indices[i];
	}

	return result;
}