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
		bool inverse(FMat4<T> &invOut) {
			FMat4<T> inv;
			T det;
			int i, j;
			T factor2233 = data[2][2] * data[3][3];
			T factor1123 = data[1][1] * data[2][3];

			inv[0][0] = data[1][1] * factor2233 - factor1123 * data[3][2] - 
						data[2][1] * data[1][2] * data[3][3] + data[2][1] * data[1][3] * data[3][2] +
						data[3][1] * data[1][2] * data[2][3] - data[3][1] * data[1][3] * data[2][2];

			inv[1][0] = -data[1][0] * factor2233 + data[1][0] * data[2][3] * data[3][2] + 
						 data[2][0] * data[1][2] * data[3][3] - data[2][0] * data[1][3] * data[3][2] - 
						 data[3][0] * data[1][2] * data[2][3] + data[3][0] * data[1][3] * data[2][2];

			inv[2][0] = data[1][0] * data[2][1] * data[3][3] - data[1][0] * data[2][3] * data[3][1] -
						data[2][0] * data[1][1] * data[3][3] + data[2][0] * data[1][3] * data[3][1] +
						data[3][0] * factor1123 - data[3][0] * data[1][3] * data[2][1];

			inv[3][0] = -data[1][0] * data[2][1] * data[3][2] + data[1][0] * data[2][2] * data[3][1] +
						 data[2][0] * data[1][1] * data[3][2] - data[2][0] * data[1][2] * data[3][1] - 
						 data[3][0] * data[1][1] * data[2][2] + data[3][0] * data[1][2] * data[2][1];

			inv[0][1] = -data[0][1] * factor2233 + data[0][1] * data[2][3] * data[3][2] + 
						 data[2][1] * data[0][2] * data[3][3] - data[2][1] * data[0][3] * data[3][2] - 
						 data[3][1] * data[0][2] * data[2][3] + data[3][1] * data[0][3] * data[2][2];

			inv[1][1] = data[0][0] * factor2233 - data[0][0] * data[2][3] * data[3][2] - 
						data[2][0] * data[0][2] * data[3][3] + data[2][0] * data[0][3] * data[3][2] + 
						data[3][0] * data[0][2] * data[2][3] - data[3][0] * data[0][3] * data[2][2];

			inv[2][1] = -data[0][0] * data[2][1] * data[3][3] + data[0][0] * data[2][3] * data[3][1] + 
						 data[2][0] * data[0][1] * data[3][3] - data[2][0] * data[0][3] * data[3][1] - 
						 data[3][0] * data[0][1] * data[2][3] + data[3][0] * data[0][3] * data[2][1];

			inv[3][1] = data[0][0] * data[2][1] * data[3][2] - data[0][0] * data[2][2] * data[3][1] - 
						data[2][0] * data[0][1] * data[3][2] + data[2][0] * data[0][2] * data[3][1] + 
						data[3][0] * data[0][1] * data[2][2] - data[3][0] * data[0][2] * data[2][1];

			inv[0][2] = data[0][1]  * data[1][2] * data[3][3] - data[0][1]  * data[1][3] * data[3][2] - 
					 data[1][1]  * data[0][2] * data[3][3] + data[1][1]  * data[0][3] * data[3][2] + 
					 data[3][1] * data[0][2] * data[1][3] - data[3][1] * data[0][3] * data[1][2];

			inv[1][2] = -data[0][0] * data[1][2] * data[3][3] + data[0][0] * data[1][3] * data[3][2] + 
					  data[1][0] * data[0][2] * data[3][3] - data[1][0] * data[0][3] * data[3][2] - 
					  data[3][0] * data[0][2] * data[1][3] + data[3][0] * data[0][3] * data[1][2];

			inv[2][2] = data[0][0]  * data[1][1] * data[3][3] - data[0][0]  * data[1][3] * data[3][1] - 
					  data[1][0]  * data[0][1] * data[3][3] + data[1][0]  * data[0][3] * data[3][1] + 
					  data[3][0] * data[0][1] * data[1][3] - data[3][0] * data[0][3] * data[1][1];

			inv[3][2] = -data[0][0] * data[1][1] * data[3][2] + data[0][0] * data[1][2] * data[3][1] + 
					   data[1][0] * data[0][1] * data[3][2] - data[1][0] * data[0][2] * data[3][1] - 
					   data[3][0] * data[0][1] * data[1][2] + data[3][0] * data[0][2] * data[1][1];

			inv[0][3] = -data[0][1] * data[1][2] * data[2][3] + data[0][1] * data[1][3] * data[2][2] + 
					  data[1][1] * data[0][2] * data[2][3] - data[1][1] * data[0][3] * data[2][2] - 
					  data[2][1] * data[0][2] * data[1][3] + data[2][1] * data[0][3] * data[1][2];

			inv[1][3] = data[0][0] * data[1][2] * data[2][3] - data[0][0] * data[1][3] * data[2][2] - 
					 data[1][0] * data[0][2] * data[2][3] + data[1][0] * data[0][3] * data[2][2] + 
					 data[2][0] * data[0][2] * data[1][3] - data[2][0] * data[0][3] * data[1][2];

			inv[2][3] = -data[0][0] * factor1123 + data[0][0] * data[1][3] * data[2][1] + 
					   data[1][0] * data[0][1] * data[2][3] - data[1][0] * data[0][3] * data[2][1] - 
					   data[2][0] * data[0][1] * data[1][3] + data[2][0] * data[0][3] * data[1][1];

			inv[3][3] = data[0][0] * data[1][1] * data[2][2] - data[0][0] * data[1][2] * data[2][1] - 
					  data[1][0] * data[0][1] * data[2][2] + data[1][0] * data[0][2] * data[2][1] + 
					  data[2][0] * data[0][1] * data[1][2] - data[2][0] * data[0][2] * data[1][1];

			det = data[0][0] * inv[0][0] + data[0][1] * inv[2][0] + data[0][2] * inv[2][0] + data[0][3] * inv[3][0];

			if (det == 0)
				return false;

			det = 1.0 / det;

			for (i = 0; i < 4; i++)
				for(j=0; j<4; j++)
					invOut[i][j] = inv[i][j] * det;

			return true;
		}
	};
}

#endif
