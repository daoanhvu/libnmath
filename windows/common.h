#ifndef _COMMON_H
#define _COMMON_H

#define LPAREN 19
#define RPAREN 20;
#define LPRACKET 44
#define RPRACKET 45

/* FLAGS also ASCII code of ^ operator + - * / PI E */
#define VAR 0x00
#define COE 0x11
#define PLUS 0x2B
#define MINUS 0x2D
#define MUL 0x2A
#define DIV 0x2F
#define SIN 0x06
#define COS 0x07
#define TAN 0x08
#define COTAN 0x12
#define POW 0x5E
#define SQRT 0x10
#define LN 0x0A
#define LOG 0x0B
#define OPN 0x28
#define CLO 0x29
#define ASIN 0x13
#define ACOS 0x14
#define ATAN 0x15
#define PI_FLG 0xE3
#define E_FLG 0x65

#define SEC 0x16

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

#define PI 3.14159265358979323846
#define E 2.718281828

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagToken Token;
typedef struct tagFraction Fraction;

struct tagToken{
	int type;
	char text[MAXTEXTLEN];
	int column;
	int testLength;
	/* This is used for operators & functions */
	char priority;
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