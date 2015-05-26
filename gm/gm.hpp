#ifndef _GM_H
#define _GM_H

#include "vec3.hpp"
#include "vec4.hpp"
#include "fmat4.hpp"
#include "quat.hpp"
#include <math.h>

namespace gm {
	typedef Vec3<float> vec3;
	typedef Vec4<float> vec4;
	typedef FMat4<float> mat4;
	typedef Quat<float> quat;

	template <typename T>
	Vec3<T> normalize(T vx, T vy, T vz) {
		T mag = sqrt(vx*vx + vy*vy + vz*vz);
		return Vec3<T>(vx/mag, vy/mag, vz/mag);
	}

	template <typename T>
	Vec3<T> normalize(Vec3<T> const &v) {
		T mag = sqrt(v.data[0]*v.data[0] + v.data[1]*v.data[1] + v.data[2]*v.data[2]);
		return Vec3<T>(v.data[0]/mag, v.data[1]/mag, v.data[2]/mag);
	}
	
	template <typename T>
	FMat4<T> rotate(T angleR, Vec4<T>  const &v) {
		Vec3<T> axis, temp;
		FMat4<T> Rotate;
		T c = cos(angleR);
		T s = sin(angleR);

		axis = normalize(v.data[0], v.data[1], v.data[2]);
		//temp = (T(1) - c) * axis; <== Why it's ambiguous here ?????
		temp = axis * (T(1) - c);
		Rotate[0][0] = c + temp[0] * axis[0];
		Rotate[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
		Rotate[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

		Rotate[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
		Rotate[1][1] = c + temp[1] * axis[1];
		Rotate[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

		Rotate[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
		Rotate[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
		Rotate[2][2] = c + temp[2] * axis[2];
		
		//Return the rotation matrix
		return Rotate;

		/*
		FMat4<T> Result;
		Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
		Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
		Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
		Result[3] = m[3];
		return Result;
		*/
	}
}

#endif