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
}

#endif