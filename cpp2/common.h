#ifndef _COMMON_H
#define _COMMON_H

/* Unicode values */
#define COMMA			0x0000002C
#define SEMI			0x0000003B
#define AND				0x00002227
#define OR				0x00002228
#define GT				0x000000E3 //>
#define LT				0x0000003C //<
#define GTE				0x00002265 //>=
#define LTE				0x00002264 //<=
#define NE				0x00002260 // not equals !=
#define EQ				0x0000003D // equals =
#define LPAREN			0x00000028
#define RPAREN			0x00000029
#define LPRACKET 		0x0000005B
#define RPRACKET 		0x0000005D
#define ELEMENT_OF 		0x00002208
#define PLUS 			0x0000002B
#define MINUS 			0x0000002D
#define MULTIPLY 		0x000000D7
#define DIVIDE 			0x000000F7
#define POWER 			0x0000005E
#define SQRT 			0x0000221A
#define CBRT 			0x0000221B
#define PI_TYPE 		0x000003C0
#define E_TYPE 			0x0000212F
#define TYPE_INFINITY	0x0000221E

/* Custom values */
#define SEC 			0x000100E2
#define DOMAIN_NOTATION	0x000100E3
#define GT_LT			0x00010010
#define GTE_LT			0x00010011
#define GT_LTE			0x00010012
#define GTE_LTE			0x00010013
#define LN 				0x000100E0
#define LOG 			0x000100E1
#define IMPLY			0x00010000
#define RARROW			0x00010001
#define SIN 			0x000100F0
#define COS 			0x000100F1
#define TAN 			0x000100F2
#define COTAN 			0x000100F3
#define ASIN 			0x000100F4
#define ACOS 			0x000100F5
#define ATAN 			0x000100F6
#define NUMBER			0x00010002
#define NAME			0x00010003
#define VARIABLE 		0x00010004

#define TYPE_FLOATING_POINT 0
#define TYPE_FRACTION 1

#define COE_VAL_PRIORITY 0x00
#define PRIORITY_0 0x00
#define PLUS_MINUS_PRIORITY 0x01
#define MUL_DIV_PRIORITY 0x02
#define FUNCTION_PRIORITY 0x03

#define NMATH_NO_ERROR 					0
#define ERROR_DIV_BY_ZERO 				-1
#define ERROR_TOO_MANY_PARENTHESE 		-2
#define ERROR_OPERAND_MISSING 			-3
#define ERROR_PARSE 					-4
#define ERROR_TOO_MANY_FLOATING_POINT 	-5
#define ERROR_PARENTHESE_MISSING 		-6
#define ERROR_OUT_OF_DOMAIN 			-7
#define ERROR_SYNTAX 					-8
#define ERROR_NOT_AN_EXPRESSION 		-9
#define ERROR_NOT_A_FUNCTION 			-10
#define ERROR_BAD_TOKEN 				-11
#define ERROR_LEXER 					-12
#define ERROR_PARSING_NUMBER 			-13
#define ERROR_MISSING_VARIABLE 			-14
#define ERROR_LOG 						-15
#define ERROR_MISSING_DOMAIN 			-16
#define E_NOT_ENOUGH_MEMORY 			-17
#define ERROR_MISSING_FUNCTION_NOTATION -18
#define ERROR_MALFORMED_ENCODING 		-19
#define E_NOT_ENOUGH_PLACE 				-20

#define LEFT_INF	0x02
#define RIGHT_INF	0x01
#define AVAILABLE	0x04

#define MAXTEXTLEN 	16
#define INCLEN 		8
#define PI			3.14159265358979323846
#define E			2.718281828

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

typedef struct tagListCriteria ListCriteria;
typedef struct tagFData FData;
typedef struct tagListFData ListFData;
typedef struct tagOutBuiltCriteria OutBuiltCriteria;
typedef struct tagFraction Fraction;
typedef struct tagToken Token;
typedef struct tagTokenList TokenList;
typedef struct tagNMAST NMAST;
typedef struct tagNMASTList NMASTList;

struct tagFraction {
	int numerator;
	int denomerator;
};

struct tagToken {
	int type;
	char text[MAXTEXTLEN];
	int column;
	unsigned char textLength;
	
	/* This is used for operators & functions */
	char priority;
};

struct tagNMAST {
	int type;

	char priority;
	/*
	 TYPE_FLOATING_POINT OR TYPE_FRACTION
	 0: floating point value
	 1: Fraction value
	 * */
	char valueType;
	double value;
	Fraction frValue;

	//if this ast is a VARIABLE and NAME
	char variable;
	
	/* this flag is just used for function cause the function cannot express its sign itself */
	/* MUST = 1 by default */
	int sign;
	struct tagNMAST *parent;
	struct tagNMAST *left;
	struct tagNMAST *right;

	char level;
};

struct tagNMASTList {
	unsigned int loggedSize;
	unsigned int size;
	struct tagNMAST **list;
};

/** ================================================================================================ */
struct tagListCriteria {
	void **list;
	unsigned int loggedSize;
	unsigned int size;
};

struct tagFData {
	float *data;
	unsigned int dataSize;
	unsigned int loggedSize;
	char dimension;
	
	int *rowInfo;
	unsigned int rowCount;
	unsigned int loggedRowCount;
};

struct tagListFData {
	FData **list;
	unsigned int loggedSize;
	unsigned int size;
};

struct tagOutBuiltCriteria {
	void *cr;
};

/* ====================================================================================================== */

void pushASTStack(NMASTList *sk, NMAST* ele);
NMAST* popASTStack(NMASTList *sk);
/* 
 * This do the primity test
 * return 1: if n is prime
 * otherwise return 0 */
int isPrime(long n);
/* Greatest Common Divisor*/
long gcd(long a, long b);
long lcm(long a, long b);
long l_cast(double val, double *fr);
int l_castF(float val, float *fr);
double parseDouble(const char *str, int start, int end, int *error);
// float parseFloat(const char *str, int start, int end, int *error);
int contains(int type, const int *aset, int len);
double logab(double a, double b, int *error);
float logabF(float a, float b, int *error);
double doCalculate(double val1, double val2, int type, int *error);
float doCalculateF(float val1, float val2, int type, int *error);
void clearTree(NMAST **prf);
void clearTreeContent(NMAST *prf);
char getPriorityOfType(int type);
int getErrorColumn();
int getErrorCode();
int isAFunctionType(int type);
int isAnOperatorType(int type);
int isFunctionOROperator(int type);
int isComparationOperator(int type);
int isConstant(int type);
int isLetter(char c);

/* 
	I use a pool to store AST node to reuse them later 
	this reduces the number of allocation operation so it speed up the app
*/
NMAST* getFromPool();
void putIntoPool(NMAST *ast);
void clearPool();

#ifdef DEBUG
int numberOfDynamicObject();
void incNumberOfDynamicObject();
void descNumberOfDynamicObject();
void descNumberOfDynamicObjectBy(int k);
#endif

#ifdef __cplusplus
}
#endif

#endif