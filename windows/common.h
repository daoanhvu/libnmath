#ifndef _COMMON_H
#define _COMMON_H

#define COMMA			1
#define SEMI				2
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
#define DOMAIN_NOTAION	39
#define GT_LT			40
#define GTE_LT			41
#define GT_LTE			42
#define GTE_LTE			42

#define TYPE_FLOATING_POINT 0
#define TYPE_FRACTION 1

#define COE_VAL_PRIORITY 0x00
#define PRIORITY_0 0x00
#define PLUS_MINUS_PRIORITY 0x01
#define MUL_DIV_PRIORITY 0x02
#define FUNCTION_PRIORITY 0x03

#define NO_ERROR 0
#define ERROR_DIV_BY_ZERO -1
#define ERROR_TOO_MANY_PARENTHESE -2
#define ERROR_OPERAND_MISSING -3
#define ERROR_PARSE -4
#define ERROR_TOO_MANY_FLOATING_POINT -5
#define ERROR_PARENTHESE_MISSING -6
#define ERROR_OUT_OF_DOMAIN -7
#define ERROR_SYNTAX -8
#define ERROR_NOT_AN_EXPRESSION -9
#define ERROR_NOT_A_FUNCTION -10
#define ERROR_BAD_TOKEN -11
#define ERROR_LEXER -12
#define ERROR_PARSING_NUMBER -13
#define ERROR_MISSING_VARIABLE -14
#define ERROR_LOG -15

#define MAXTEXTLEN 20
#define INCLEN 10

#define PI	3.14159265358979323846
#define E	2.718281828

#ifndef TRUE
	#define TRUE -1
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL ((void*)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagFraction Fraction;
typedef struct tagToken Token;
typedef struct tagTokenList TokenList;
typedef struct tagNMAST NMAST;
typedef struct tagNMASTList NMASTList;

struct tagFraction{
	int numerator;
	int denomerator;
};


typedef struct tagFunct{
	char *str;
	int len;

	char variable[3];
	char valLen;

	NMAST **prefix;
	int prefixLen;
	int prefixAllocLen;

	NMAST **variableNode;
	int numVarNode;
	
	NMAST *domain;
} Function;

struct tagToken{
	int type;
	char text[MAXTEXTLEN];
	int column;
	int textLength;
	
	/* This is used for operators & functions */
	char priority;
};

struct tagTokenList{
	int loggedSize;
	int size;
	struct tagToken **list;
};

struct tagNMAST{
	int type;

	int priority;
	/*
	 TYPE_FLOATING_POINT OR TYPE_FRACTION
	 0: floating point value
	 1: Fraction value
	 * */
	char valueType;
	double value;
	Fraction frValue;

	//if this ast is a VARIABLE
	char variable;
	
	/* this flag is just used for function cause the function cannot express its sign itself */
	/* MUST = 1 by default */
	int sign;
	struct tagNMAST *parent;
	struct tagNMAST *left;
	struct tagNMAST *right;
};

struct tagNMASTList{
	int loggedSize;
	int size;
	struct tagNMAST **list;
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
int contains(int type, const int *aset, int len);
double logab(double a, double b, int *error);
double doCalculate(double val1, double val2, int type, int *error);
void clearTree(NMAST **prf);
int getErrorColumn();
int getErrorCode();
int isAFunctionType(int type);
int isAnOperatorType(int type);
int isFunctionOROperator(int type);
int isComparationOperator(int type);
int isConstant(int type);
int isLetter(char c);

#ifdef __cplusplus
}
#endif

#endif