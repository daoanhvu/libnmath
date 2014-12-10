#ifndef _COMPLEX_H_
#define _COMPLEX_H_

/*
 * z = image*i + real
 * i^2 = -1
 * */
 
namespace nmath {
	class Complex {
		private:
			double real;
			double image;
			
		public:
			Complex(double r, double img);
			~Complex();

			Complex& operator =(const Complex& c);

			friend Complex& operator +(double r, const Complex& c);
	};
}
#endif
