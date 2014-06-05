#include "nmbase.h"
#include <stdlib.h>
#include <math.h>

int isPrime(long n){
	long i, sq;
	if(n<2) return 0;
	if(n==2) return 1;
	sq = sqrt(n);
	for(i=2;i<=sq;i++)
		if( (n%i) == 0)
			return 0;
	return 1;
}

/* Greatest Common Divisor*/
long gcd(long a, long b){
	long c;
	while(a !=0 ){
		c = a;
		a = b % a;
		b = c;
	}
	return b;
}

long lcm(long a, long b){
	return (a*b)/gcd(a, b);
}

double parseDouble(char *str, int start, int end, int *error){
	int i;
	double val = 0;
	int isFloatingPoint = 0;
	long floating = 1;
	int negative = 1;

	*error = -1;
	if(str == NULL)
		return 0;
		
	if(str[start] == '-'){
		negative = -1;
		start++;
	}

	for(i=start; i<end; i++){

		if(str[i]=='\0')
			return 0;

		if((str[i]<48) || (str[i]>57)){
			if( str[i] == 46 && isFloatingPoint==0)
				isFloatingPoint = 1;
			else{
				*error = ERROR_PARSE;
				/*printf(" Floating point ERROR F\n");*/
				return 0;
			}
		}else{
			if(isFloatingPoint){
				floating *= 10;
				val = val + (str[i] - 48)*1.0/floating;
			}else
				val = val * 10 + (str[i] - 48);
		}
	}
	(*error) = 0;
	return val*negative;
}

long l_cast(double val, double *fr){
	(*fr) = val - (long)val;
	return (long)val;
}
