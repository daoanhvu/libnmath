#ifndef _VEC3_H
#define _VEC3_H

#include <stdlib.h>

namespace gm {

	template <typename T>
	struct Vec4{
	private:
		T data[3];
	public:

		Vec4() {
			data[0] = 0;
			data[1] = 0;
			data[2] = 0;
		}

		Vec4(T v0, T v1, T v2) {
			data[0] = v0;
			data[1] = v1;
			data[2] = v2;
		}

		T& operator [](int index)	{ return data[index]; }

		Vec3<T>& operator =(Vec3<T> const &v) {
			//memcpy could be faster
			memcpy(&data, &v.data, 3 * sizeof(T));
			return *this;
		}

		Vec3<T> operator +(Vec3<T> &v2) {
			return Vec3<T>(data[0] + v2[0],
				data[1] + v2[1],
				data[2] + v2[2]);
		}

		//DOT product
		Vec3<T> operator *(FVec4<T> &v2) {
			return FVec4<T>();
		}

		Vec3<T> operator +(T a) {
			return Vec3<T>(data[0] + a, data[1] + a, data[2] + a);
		}

		Vec3<T> operator *(T a) {
			return Vec3<T>(data[0] * a, data[1] * a, data[2] * a);
		}

		Vec3<T>& operator /=(T a) {
			data[0] /= a;
			data[1] /= a;
			data[2] /= a;

			return *this;
		}
	};

	template <typename T>
	inline T dot(Vec3 &v1, Vec3 &v2) {
		return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
	}

	inline Vec3 cross(Vec3 &v1, Vec3 &v2) {
		return Vec3(v1[1]*v2[2]-v1[2]*v2[1],
					v1[2]*v2[0]-v1[0]*v2[2],
					v1[0]*v2[1]-v1[1]*v2[0]);
	}
}