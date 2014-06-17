#include <stdlib.h>
#include <math.h>
#include "common.h"

#define TRUE -1
#define FALSE 0

const int FUNCTIONS[] = {SIN, COS, TAN, COTAN, ASIN, ACOS, ATAN, LOG, LN, SQRT};
const int FUNCTION_COUNT = 10;

const int OPERATORS[] = {PLUS,MINUS,MULTIPLY,DIVIDE,POWER};
const int OPERATOR_COUNT = 5;

const int COMPARING_OPERATORS[] = {LT,LTE,EQ,GT,GTE};
const int COMPARING_OPERATORS_COUNT = 5;

int isPrime(long n){
	long i, sq;
	if(n<2) return 0;
	if(n==2) return 1;
	sq = (long) sqrt((double)n);
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

int contains(int type, const int *aset, int len){
	int i;
	for(i=0; i<len; i++)
		if(type == aset[i])
			return TRUE;
	return FALSE;
}

double logab(double a, double b, int *error){
	(*error) = 0;
	if( (b > 0.0) && (a > 0.0) && (a != 1.0))
		return log(b)/log(a);

	(*error) = ERROR_LOG;
	return 0;
}

double doCalculate(double val1, double val2, int type, int *error){
	(*error) = 0;
	switch(type){
		case PLUS:
			/*printf("%lf+%lf=%lf\n", val1, val2, val1 + val2);*/
			return val1 + val2;

		case MINUS:
			/*printf("%lf-%lf=%lf\n", val1, val2, val1 - val2);*/
			return val1 - val2;

		case MULTIPLY:
			/*printf("%lf*%lf=%lf\n", val1, val2, val1 * val2);*/
			return val1 * val2;

		case DIVIDE:
			if(val2 == 0.0){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return val1/val2;

		case POWER:
			return pow(val1, val2);

		case LOG:
			return logab(val1, val2, error);

		case LN:
			return log(val2);

		case SIN:
			return sin(val2);

		case ASIN:
			return asin(val2);

		case COS:
			return cos(val2);

		case ACOS:
			return acos(val2);

		case COTAN:
			if(val2==0 || val2 == PI){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return cos(val2)/sin(val2);

		case TAN:
			if(val2==PI/2){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return tan(val2);

		case ATAN:
			return atan(val2);
		
		case SEC:
			if(val2==PI/2){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return 1/cos(val2);

		case SQRT:
			if(val2 < 0){
				(*error) = ERROR_OUT_OF_DOMAIN;
				return 0;
			}
			return sqrt(val2);
	}
	return 0;
}

int isAFunctionType(int type){
	int i;
	for(i=0; i<FUNCTION_COUNT; i++)
		if(type == FUNCTIONS[i])
			return TRUE;
	return FALSE;
}

int isAnOperatorType(int type){
	int i;
	for(i=0; i<OPERATOR_COUNT; i++)
		if(type == OPERATORS[i])
			return TRUE;
	return FALSE;
}

int isFunctionOROperator(int type){
	int i;
	for(i=0; i<FUNCTION_COUNT; i++)
		if(type == FUNCTIONS[i])
			return TRUE;

	for(i=0; i<OPERATOR_COUNT; i++)
		if(type == OPERATORS[i])
			return TRUE;

	return FALSE;
}

int isComparationOperator(int type){
	int i;
	for(i=0; i<COMPARING_OPERATORS_COUNT; i++)
		if(type == COMPARING_OPERATORS[i])
			return TRUE;
	return FALSE;
}

int isLetter(char c){
	if (( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ))
		return TRUE;
	return FALSE;
}

int isConstant(int type){
	if(type == NUMBER || type == PI_TYPE || type == E_TYPE)
		return TRUE;
	return FALSE;
}