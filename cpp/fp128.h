#ifndef _FP128_H
#define _FP128_H

/*
	To calculate the bias for an arbitrary sized floating point number apply the formula 2k−1 − 1 where k is the number of bits in the exponent.[1]

When interpreting the floating-point number, the bias is subtracted to retrieve the actual exponent.

For a single-precision number, an exponent in the range −126 .. +127 is biased by adding 127 to get a value in the range 1 .. 254 (0 and 255 have special meanings).
For a double-precision number, an exponent in the range −1022 .. +1023 is biased by adding 1023 to get a value in the range 1 .. 2046 (0 and 2047 have special meanings).
For a quad-precision number, an exponent in the range −16382 .. +16383 is biased by adding 16383 to get a value in the range 1 .. 32766 (0 and 32767 have special meanings).

Referrences:
O'Hallaron, Randal E. Bryant, David R. (2010). Computer systems : a programmer's perspective (2nd ed. ed.). Boston: Prentice Hall. ISBN 978-0-13-610804-7.

	Here I use 21 bit for exponent
*/
#define BIAS 1048575

class FP128 {
	private:
		unsigned char data[16];

	public:
		FP128();
		FP128(double value);
};

#endif