#ifndef _FMAT4_H
#define _FMAT4_H

#include <stdlib.h>

namespace gm {

	template <typename T>
	struct FVec4{
	private:
		T data[4];
	public:

		FVec4() {
			data[0] = 0;
			data[1] = 0;
			data[2] = 0;
			data[3] = 0;
		}

		FVec4(T v0, T v1, T v2, T v3) {
			data[0] = v0;
			data[1] = v1;
			data[2] = v2;
			data[3] = v3;
		}

		T& operator [](int index)	{ return data[index]; }

		FVec4<T>& operator =(FVec4<T> const &m) {
			//memcpy could be faster
			memcpy(&data, &m.data, 4 * sizeof(T));
			return *this;
		}

		FVec4<T> operator +(FVec4<T> &v2) {
			return FVec4<T>(data[0] + v2[0],
				data[1] + v2[1],
				data[2] + v2[2],
				data[3] + v2[3]);
		}

		//DOT product
		FVec4<T> operator *(FVec4<T> &v2) {
			return FVec4<T>();
		}

		FVec4<T> operator +(T a) {
			return FVec4<T>(data[0] + a, data[1] + a, data[2] + a, data[3] + a);
		}

		FVec4<T> operator *(T a) {
			return FVec4<T>(data[0] * a, data[1] * a, data[2] * a, data[3] * a);
		}

		FVec4<T>& operator /=(T a) {
			data[0] /= a;
			data[1] /= a;
			data[2] /= a;
			data[3] /= a;

			return *this;
		}
	};

	template <typename T>
	struct FMat4 {
	private:
		FVec4<T> data[4];

	public:
		FMat4() {
			data[0][0] = 1;
			data[0][1] = 0;
			data[0][2] = 0;
			data[0][3] = 0;

			data[1][0] = 0;
			data[1][1] = 1;
			data[1][2] = 0;
			data[1][3] = 0;

			data[2][0] = 0;
			data[2][1] = 0;
			data[2][2] = 1;
			data[2][3] = 0;

			data[3][0] = 0;
			data[3][1] = 0;
			data[3][2] = 0;
			data[3][3] = 1;
		}

		~FMat4() {}

		void setIdentity() {
			data[0][0] = 1;
			data[0][1] = 0;
			data[0][2] = 0;
			data[0][3] = 0;

			data[1][0] = 0;
			data[1][1] = 1;
			data[1][2] = 0;
			data[1][3] = 0;

			data[2][0] = 0;
			data[2][1] = 0;
			data[2][2] = 1;
			data[2][3] = 0;

			data[3][0] = 0;
			data[3][1] = 0;
			data[3][2] = 0;
			data[3][3] = 1;
		}

		FVec4<T>& operator [](int index)	{ return data[index]; }

		FMat4<T>& operator =(FMat4<T> const &m) {
			//memcpy could be faster
			memcpy(&data, &m.data, 16 * sizeof(T));
			//data[0] = m[0];
			//data[1] = m[1];
			//data[2] = m[2];
			//data[3] = m[3];
			return *this;
		}

		FMat4<T> operator *(FMat4<T> &m2) {
			FMat4<T> result;
			int i, j, k;
			T s;

			for(i=0; i<4; i++) {
				for(j=0; j<4; j++) {
					s = 0;
					for(k=0; k<4; k++){
						s += data[k][j] * m2[j][k];
					}

					result[i][j] = s;
				}
			}
			return result;
		}

		FVec4<T> operator *(FVec4<T> &v) {
			FVec4<T> result;
			int i, j, k;
			T s;
			for(i=0; i<4; i++) {
				s = 0;
				for(j=0; j<4; j++)
					s += data[i][j] * v[j];
				
				result[i] = s;
			}
			
			return result;
		}
	};
}

#endif
