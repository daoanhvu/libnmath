#ifndef _FRACTION_H
#define _FRACTION_H

/*
	Author: Dao Anh Vu
*/
namespace nmath {
	class Fraction {
		public:
			int numerator;
			int denomerator;
			
		public:
			Fraction();
			Fraction(int n, int d);
			~Fraction()	{}
			
			double getValue() { return (1.0*numerator)/denomerator; }
			
			Fraction& operator +=(Fraction& c);
			Fraction& operator -=(Fraction& c);
			Fraction& operator +=(int c);
			Fraction& operator -=(int c);
			Fraction& operator =(const Fraction &);

			friend Fraction operator +(int n, const Fraction& f);
			friend Fraction& operator -(int n, const Fraction& f);
			friend Fraction& operator *(int n, const Fraction& f);
			friend Fraction& operator /(int n, const Fraction& f);

			friend Fraction operator +(const Fraction& f, int n);
			friend Fraction& operator -(const Fraction& f, int n);
			friend Fraction& operator *(const Fraction& f, int n);
			friend Fraction& operator /(const Fraction& f, int n);
	};
}

#endif