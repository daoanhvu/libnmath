#ifndef _NMBASE
#define _NMBASE

#define ERROR_PARSE -4

typedef struct tagFraction{
	int numerator;
	int denomerator;
}Fraction;

/* 
 * This do the primity test
 * return 1: if n is prime
 * otherwise return 0 */
int isPrime(long n);
/* Greatest Common Divisor*/
long gcd(long a, long b);
long lcm(long a, long b);
long l_cast(double val, double *fr);
double parseDouble(char *str, int start, int end, int *error);



#endif
