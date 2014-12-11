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
			
			friend Complex& operator +(double r, const Complex& c);
			friend Complex& operator +(const Complex& c, double r);
			friend Complex& operator +(const Complex& c1, const Complex& c2);
	};
}
#endif
