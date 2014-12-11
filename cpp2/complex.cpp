#include "complex.h"

using namespace nmath;

Complex::Complex(double r, double img): real(r), image(img) {
}

Complex::~Complex() {
}

Complex& Complex::operator =(const Complex& c){
	this->real = c.real;
	this->image = c.image;

	return *this;
}

Complex& operator +(double r, const Complex& c) {
	Complex result(0, 0);
	return result;
}