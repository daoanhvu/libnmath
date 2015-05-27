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
		//Column-major
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
			int i;
			
			result[0][0] = 0;
			result[1][1] = 0;
			result[2][2] = 0;
			result[3][3] = 0;
			for(i=0; i<4; i++) {
				result[0][0] += data[i][0] * m2[0][i];
				result[0][1] += data[i][1] * m2[0][i];
				result[0][2] += data[i][2] * m2[0][i];
				result[0][3] += data[i][3] * m2[0][i];
				
				result[1][0] += data[i][0] * m2[1][i];
				result[1][1] += data[i][1] * m2[1][i];
				result[1][2] += data[i][2] * m2[1][i];
				result[1][3] += data[i][3] * m2[1][i];
				
				result[2][0] += data[i][0] * m2[2][i];
				result[2][1] += data[i][1] * m2[2][i];
				result[2][2] += data[i][2] * m2[2][i];
				result[2][3] += data[i][3] * m2[2][i];
				
				result[3][0] += data[i][0] * m2[3][i];
				result[3][1] += data[i][1] * m2[3][i];
				result[3][2] += data[i][2] * m2[3][i];
				result[3][3] += data[i][3] * m2[3][i];
			}
			
			return result;
		}

		Vec4<T> operator *(Vec4<T> &v) {
			Vec4<T> result;
			int i, j;
			for(i=0; i<4; i++) {
				for(j=0; j<4; j++)
					result[i] += data[j][i] * v[j];
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
		
		/*
			http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche23.html
		*/
		bool inverse(FMat4<T> &outM) {
			T det;
			
			outM[0][0] = m[5]  * m[10] * m[15] - 
					 m[5]  * m[11] * m[14] - 
					 m[9]  * m[6]  * m[15] + 
					 m[9]  * m[7]  * m[14] +
					 m[13] * m[6]  * m[11] - 
					 m[13] * m[7]  * m[10];

			outM[4] = -m[4]  * m[10] * m[15] + 
					  m[4]  * m[11] * m[14] + 
					  m[8]  * m[6]  * m[15] - 
					  m[8]  * m[7]  * m[14] - 
					  m[12] * m[6]  * m[11] + 
					  m[12] * m[7]  * m[10];

			outM[8] = m[4]  * m[9] * m[15] - 
					 m[4]  * m[11] * m[13] - 
					 m[8]  * m[5] * m[15] + 
					 m[8]  * m[7] * m[13] + 
					 m[12] * m[5] * m[11] - 
					 m[12] * m[7] * m[9];

			outM[12] = -m[4]  * m[9] * m[14] + 
					   m[4]  * m[10] * m[13] +
					   m[8]  * m[5] * m[14] - 
					   m[8]  * m[6] * m[13] - 
					   m[12] * m[5] * m[10] + 
					   m[12] * m[6] * m[9];

			outM[1] = -m[1]  * m[10] * m[15] + 
					  m[1]  * m[11] * m[14] + 
					  m[9]  * m[2] * m[15] - 
					  m[9]  * m[3] * m[14] - 
					  m[13] * m[2] * m[11] + 
					  m[13] * m[3] * m[10];

			outM[5] = m[0]  * m[10] * m[15] - 
					 m[0]  * m[11] * m[14] - 
					 m[8]  * m[2] * m[15] + 
					 m[8]  * m[3] * m[14] + 
					 m[12] * m[2] * m[11] - 
					 m[12] * m[3] * m[10];

			outM[9] = -m[0]  * m[9] * m[15] + 
					  m[0]  * m[11] * m[13] + 
					  m[8]  * m[1] * m[15] - 
					  m[8]  * m[3] * m[13] - 
					  m[12] * m[1] * m[11] + 
					  m[12] * m[3] * m[9];

			outM[13] = m[0]  * m[9] * m[14] - 
					  m[0]  * m[10] * m[13] - 
					  m[8]  * m[1] * m[14] + 
					  m[8]  * m[2] * m[13] + 
					  m[12] * m[1] * m[10] - 
					  m[12] * m[2] * m[9];

			outM[2] = m[1]  * m[6] * m[15] - 
					 m[1]  * m[7] * m[14] - 
					 m[5]  * m[2] * m[15] + 
					 m[5]  * m[3] * m[14] + 
					 m[13] * m[2] * m[7] - 
					 m[13] * m[3] * m[6];

			outM[6] = -m[0]  * m[6] * m[15] + 
					  m[0]  * m[7] * m[14] + 
					  m[4]  * m[2] * m[15] - 
					  m[4]  * m[3] * m[14] - 
					  m[12] * m[2] * m[7] + 
					  m[12] * m[3] * m[6];

			outM[10] = m[0]  * m[5] * m[15] - 
					  m[0]  * m[7] * m[13] - 
					  m[4]  * m[1] * m[15] + 
					  m[4]  * m[3] * m[13] + 
					  m[12] * m[1] * m[7] - 
					  m[12] * m[3] * m[5];

			outM[14] = -m[0]  * m[5] * m[14] + 
					   m[0]  * m[6] * m[13] + 
					   m[4]  * m[1] * m[14] - 
					   m[4]  * m[2] * m[13] - 
					   m[12] * m[1] * m[6] + 
					   m[12] * m[2] * m[5];

			outM[3] = -m[1] * m[6] * m[11] + 
					  m[1] * m[7] * m[10] + 
					  m[5] * m[2] * m[11] - 
					  m[5] * m[3] * m[10] - 
					  m[9] * m[2] * m[7] + 
					  m[9] * m[3] * m[6];

			outM[7] = m[0] * m[6] * m[11] - 
					 m[0] * m[7] * m[10] - 
					 m[4] * m[2] * m[11] + 
					 m[4] * m[3] * m[10] + 
					 m[8] * m[2] * m[7] - 
					 m[8] * m[3] * m[6];

			outM[11] = -m[0] * m[5] * m[11] + 
					   m[0] * m[7] * m[9] + 
					   m[4] * m[1] * m[11] - 
					   m[4] * m[3] * m[9] - 
					   m[8] * m[1] * m[7] + 
					   m[8] * m[3] * m[5];

			outM[3][3] = m[0] * m[5] * m[10] - 
					  m[0] * m[6] * m[9] - 
					  m[4] * m[1] * m[10] + 
					  m[4] * m[2] * m[9] + 
					  m[8] * m[1] * m[6] - 
					  m[8] * m[2] * m[5];

			det = m[0] * outM[0] + m[1] * outM[4] + m[2] * outM[8] + m[3] * outM[12];

			if (det == 0)
				return false;

			det = 1.0 / det;

			for (i = 0; i < 16; i++)
				outM[i] = outM[i] * det;

			return true;
		}
	};
}

#endif
