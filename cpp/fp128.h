#ifndef _FP128_H
#define _FP128_H

/*
	To calculate the bias for an arbitrary sized floating point number apply the formula 2k−1 − 1 where k is the number of bits in the exponent.

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