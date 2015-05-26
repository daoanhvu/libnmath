#ifndef _FMAT4_H
#define _FMAT4_H

#include <stdlib.h>
#ifdef _WIN32
#include <memory.h>
#endif
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
						s += data[i][k] * m2[k][j];
					}

					result[i][j] = s;
				}
			}
			return result;
		}

		Vec4<T> operator *(Vec4<T> &v) {
			Vec4<T> result;
			int i, j;
			T s;
			for(i=0; i<4; i++) {
				s = 0;
				for(j=0; j<4; j++)
					//s += data[i][j] * v[j];
					s += data[j][i] * v[j];
				
				result[i] = s;
			}
			
			return result;
		}
		
		void operator /=(T divisor) {			
			int i, j;
			for(i=0; i<4; i++) {
				for(j=0; j<4; j++)
					data[j][i] = data[j][i] / divisor;
			}
		}
		
		bool inverse(FMat4<T> &outM) {
			T subFactor00 = data[2][2] * data[3][3] - data[3][2] * data[2][3];
			T subFactor01 = data[2][1] * data[3][3] - data[3][1] * data[2][3];
			T subFactor02 = data[2][1] * data[3][2] - data[3][1] * data[2][2];
			T subFactor03 = data[2][0] * data[3][3] - data[3][0] * data[2][3];
			T subFactor04 = data[2][0] * data[3][2] - data[3][0] * data[2][2];
			T subFactor05 = data[2][0] * data[3][1] - data[3][0] * data[2][1];
			T subFactor06 = data[1][2] * data[3][3] - data[3][2] * data[1][3];
			T subFactor07 = data[1][1] * data[3][3] - data[3][1] * data[1][3];
			T subFactor08 = data[1][1] * data[3][2] - data[3][1] * data[1][2];
			T subFactor09 = data[1][0] * data[3][3] - data[3][0] * data[1][3];
			T subFactor10 = data[1][0] * data[3][2] - data[3][0] * data[1][2];
			T subFactor11 = data[1][1] * data[3][3] - data[3][1] * data[1][3];
			T subFactor12 = data[1][0] * data[3][1] - data[3][0] * data[1][1];
			T subFactor13 = data[1][2] * data[2][3] - data[2][2] * data[1][3];
			T subFactor14 = data[1][1] * data[2][3] - data[2][1] * data[1][3];
			T subFactor15 = data[1][1] * data[2][2] - data[2][1] * data[1][2];
			T subFactor16 = data[1][0] * data[2][3] - data[2][0] * data[1][3];
			T subFactor17 = data[1][0] * data[2][2] - data[2][0] * data[1][2];
			T subFactor18 = data[1][0] * data[2][1] - data[2][0] * data[1][1];
			
			outM[0][0] = + (data[1][1] * subFactor00 - data[1][2] * subFactor01 + data[1][3] * subFactor02);
			outM[0][1] = - (data[1][0] * subFactor00 - data[1][2] * subFactor03 + data[1][3] * subFactor04);
			outM[0][2] = + (data[1][0] * subFactor01 - data[1][1] * subFactor03 + data[1][3] * subFactor05);
			outM[0][3] = - (data[1][0] * subFactor02 - data[1][1] * subFactor04 + data[1][2] * subFactor05);

			outM[1][0] = - (data[0][1] * subFactor00 - data[0][2] * subFactor01 + data[0][3] * subFactor02);
			outM[1][1] = + (data[0][0] * subFactor00 - data[0][2] * subFactor03 + data[0][3] * subFactor04);
			outM[1][2] = - (data[0][0] * subFactor01 - data[0][1] * subFactor03 + data[0][3] * subFactor05);
			outM[1][3] = + (data[0][0] * subFactor02 - data[0][1] * subFactor04 + data[0][2] * subFactor05);

			outM[2][0] = + (data[0][1] * subFactor06 - data[0][2] * subFactor07 + data[0][3] * subFactor08);
			outM[2][1] = - (data[0][0] * subFactor06 - data[0][2] * subFactor09 + data[0][3] * subFactor10);
			outM[2][2] = + (data[0][0] * subFactor11 - data[0][1] * subFactor09 + data[0][3] * subFactor12);
			outM[2][3] = - (data[0][0] * subFactor08 - data[0][1] * subFactor10 + data[0][2] * subFactor12);

			outM[3][0] = - (data[0][1] * subFactor13 - data[0][2] * subFactor14 + data[0][3] * subFactor15);
			outM[3][1] = + (data[0][0] * subFactor13 - data[0][2] * subFactor16 + data[0][3] * subFactor17);
			outM[3][2] = - (data[0][0] * subFactor14 - data[0][1] * subFactor16 + data[0][3] * subFactor18);
			outM[3][3] = + (data[0][0] * subFactor15 - data[0][1] * subFactor17 + data[0][2] * subFactor18);

			T determinant = 
				+ data[0][0] * outM[0][0] 
				+ data[0][1] * outM[0][1] 
				+ data[0][2] * outM[0][2] 
				+ data[0][3] * outM[0][3];

			outM /= determinant;
			
			return true;
		}
	};
}

#endif
