#ifndef _FMAT4_H
#define _FMAT4_H

#include <stdlib.h>
#include "vec4.hpp"

namespace gm {
	template <typename T>
	struct FMat4 {
	private:
		Vec4<T> data[4];

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

		FMat4(T value) {
			int i, j;

			//could use memset
			for(i=0; i<4; i++)
				for(j=0; j<4; j++)
					data[i][j] = value;
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

		Vec4<T>& operator [](int index)	{ return data[index]; }

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

		Vec4<T> operator *(Vec4<T> &v) {
			Vec4<T> result;
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
