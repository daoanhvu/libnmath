#ifndef _FRACTION_H
#define _FRACTION_H

/*
	This class denote a combining criteria that consists of criterias for those hold variable that are different each other.
	Example:
		(0 < x < 1) AND (y>0): this case we got a combined criteria that consists of two criteria, one is (0 < x < 1) 
		and the another is (y>0)
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

			Fraction& operator +=(Fraction& c);
			Fraction& operator -=(Fraction& c);

			Fraction& operator =(const Fraction &);

			friend Fraction& operator +(int n, const Fraction& f);
			friend Fraction& operator -(int n, const Fraction& f);
			friend Fraction& operator *(int n, const Fraction& f);
			friend Fraction& operator /(int n, const Fraction& f);

			friend Fraction& operator +(const Fraction& f, int n);
			friend Fraction& operator -(const Fraction& f, int n);
			friend Fraction& operator *(const Fraction& f, int n);
			friend Fraction& operator /(const Fraction& f, int n);
	};
}

#endif