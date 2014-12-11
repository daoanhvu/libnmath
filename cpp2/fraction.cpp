#include "fraction.h"

using namespace nmath;

Fraction::Fraction(): numerator(0), denomerator(1) {
}

Fraction::Fraction(int n, int d): numerator(n), denomerator(d){
}

Fraction& Fraction::operator +=(Fraction& c) {
	return *this;
}

Fraction& Fraction::operator -=(Fraction& c) {
	return *this;
}

Fraction& Fraction::operator =(const Fraction &c) {
	this->numerator = c.numerator;
	this->denomerator = c.denomerator;

	return *this;
}

/*
friend Fraction& operator +(int n, const Fraction& f);
friend Fraction& operator -(int n, const Fraction& f);
friend Fraction& operator *(int n, const Fraction& f);
friend Fraction& operator /(int n, const Fraction& f);
friend Fraction& operator +(const Fraction& f, int n);
friend Fraction& operator -(const Fraction& f, int n);
friend Fraction& operator *(const Fraction& f, int n);
friend Fraction& operator /(const Fraction& f, int n);
*/