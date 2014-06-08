#ifndef _COMMON_H
#define _COMMON_H

#define COMMA			1
#define SEMI			2
#define AND				3
#define OR				4
#define GT				5 //>
#define LT				6 //<
#define GTE				7 //>=
#define LTE				8 //<=
#define NE				9 // not equals !=
#define EQ				10 // equals =
#define LPAREN			11
#define RPAREN			12
#define LPRACKET 		13
#define RPRACKET 		14
#define IMPLY			15
#define RARROW			16
#define ELEMENT_OF 		17
#define NUMBER			18
#define NAME			19
#define VARIABLE 		20
#define PLUS 			21
#define MINUS 			22
#define MULTIPLY 		23
#define DIVIDE 			24
#define POWER 			25
#define SIN 			26
#define COS 			27
#define TAN 			28
#define COTAN 			29
#define ASIN 			30
#define ACOS 			31
#define ATAN 			32
#define SQRT 			33
#define LN 				34
#define LOG 			35
#define PI_TYPE 		36
#define E_TYPE 			37
#define SEC 			38

/* Function value */
#define F_COE 0x00
#define F_VAR 0x01
#define F_FUNCT 0x02
#define F_OPT 0x03
#define F_PARENT 0x04
#define F_STR 0x05
#define F_CONSTAN 0x06

#define TYPE_FLOATING_POINT 0
#define TYPE_FRACTION 1

#define COE_VAL_PRIORITY 0x00
#define PRIORITY_0 0x00
#define PLUS_MINUS_PRIORITY 0x01
#define MUL_DIV_PRIORITY 0x02
#define FUNCTION_PRIORITY 0x03

#define ERROR_DIV_BY_ZERO -1
#define ERROR_LOG -2;
#define ERROR_OPERAND_MISSING -3
#define ERROR_PARSE -4
#define ERROR_SIN -5
#define ERROR_PARENTHESE_MISSING -6
#define ERROR_OUT_OF_DOMAIN -7
#define ERROR_SYNTAX -8

#define ERROR_SIN_SQRT -9
#define ERROR_ASIN -10

#define MAXTEXTLEN 20

#define PI	3.14159265358979323846
#define E	2.718281828

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagToken Token;
typedef struct tagFraction Fraction;
typedef struct tagTokenList TokenList;

struct tagToken{
	int type;
	char text[MAXTEXTLEN];
	int column;
	int testLength;
	
	/* This is used for operators & functions */
	char priority;
};

struct tagTokenList{
	int loggedSize;
	int size;
	struct tagToken **list;
};

struct tagFraction{
	int numerator;
	int denomerator;
};

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

#ifdef __cplusplus
}
#endif

#endif