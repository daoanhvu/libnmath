#ifndef _COMMON_H
#define _COMMON_H

#define COMMA			1
#define SEMI			2
#define AND				0x00002227
#define OR				0x00002228
#define GT				5 //>
#define LT				6 //<
#define GTE				0x00002265 //>=
#define LTE				0x00002264 //<=
#define NE				0x00002260 // not equals !=
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
#define DIVIDE 			0x000000F7
#define POWER 			25
#define SIN 			26
#define COS 			27
#define TAN 			28
#define COTAN 			29
#define ASIN 			30
#define ACOS 			31
#define ATAN 			32
#define SQRT 			0x0000221A
#define LN 				34
#define LOG 			35
#define PI_TYPE 		0x000003C0
#define E_TYPE 			0x0000212F
#define SEC 			38
#define DOMAIN_NOTATION	39
#define GT_LT			40
#define GTE_LT			41
#define GT_LTE			42
#define GTE_LTE			43
#define TYPE_INFINITY	0x0000221E

#define TYPE_FLOATING_POINT 0
#define TYPE_FRACTION 1

#define COE_VAL_PRIORITY 0x00
#define PRIORITY_0 0x00
#define PLUS_MINUS_PRIORITY 0x01
#define MUL_DIV_PRIORITY 0x02
#define FUNCTION_PRIORITY 0x03

#define NMATH_NO_ERROR 0
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
#define ERROR_MISSING_DOMAIN -16
#define E_NOT_ENOUGH_MEMORY -17
#define ERROR_MISSING_FUNCTION_NOTATION -18
#define ERROR_MALFORMED_ENCODING -19
#define E_NOT_ENOUGH_PLACE -20

#define SIMPLE_CRITERIA 		0
#define COMBINED_CRITERIA 		1
#define COMPOSITE_CRITERIA 		2

#define LEFT_INF	0x02
#define RIGHT_INF	0x01
#define AVAILABLE	0x04

#define MAXTEXTLEN 16
#define INCLEN 8
#define DATA_TYPE_FP double
#define ZERO_FP	0.0
#define ONE_FP	1.0
#define PI		3.14159265358979323846
#define E		2.718281828

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

typedef struct tagCriteria Criteria;
typedef struct tagCombinedCriteria CombinedCriteria;
typedef struct tagCompositeCriteria CompositeCriteria;
typedef struct tagListCriteria ListCriteria;
typedef struct tagFData FData;
typedef struct tagListFData ListFData;
typedef struct tagOutBuiltCriteria OutBuiltCriteria;
typedef struct tagFraction Fraction;
typedef struct tagToken Token;
typedef struct tagTokenList TokenList;
typedef struct tagNMAST NMAST;
typedef struct tagNMASTList NMASTList;

//Function poiters
typedef void (*FPGetIntervalF)(const void *, const DATA_TYPE_FP *, int, void *);
typedef int (*FPCheck)(const void *, DATA_TYPE_FP *, int);

struct tagFraction {
	int numerator;
	int denomerator;
};


typedef struct tagFunct {
	char *str;
	unsigned int len;

	char variable[4];
	char valLen;

	NMASTList *prefix;
	NMASTList *domain;
	ListCriteria *criterias;

	NMAST **variableNode;
	int numVarNode;
} Function;

struct tagToken {
	int type;
	char text[MAXTEXTLEN];
	int column;
	unsigned char textLength;
	
	/* This is used for operators & functions */
	char priority;
};

struct tagTokenList {
	unsigned int loggedSize;
	unsigned int size;
	struct tagToken *list;
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
	DATA_TYPE_FP value;
	Fraction frValue;

	//if this ast is a VARIABLE and NAME
	char variable;
	
	/* this flag is just used for function cause the function cannot express its sign itself */
	/* MUST = 1 by default */
	char sign;
	struct tagNMAST *parent;
	struct tagNMAST *left;
	struct tagNMAST *right;
};

struct tagNMASTList {
	unsigned int loggedSize;
	unsigned int size;
	struct tagNMAST **list;
};

/** ================================================================================================ */

/**
	This is a single continuous interval for one variable
	Example:
		variable = 'x' and type=GT_LT we read it out like this:
		x is greater than leftVal and x is less than rightValue
*/
struct tagCriteria {
	char objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	
	/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
	int type;
	char variable;
	DATA_TYPE_FP leftVal;
	DATA_TYPE_FP rightVal;
	
	/*
		bit 0: right infinity
		bit 1: left infinity
		bit 2: available
	*/
	char flag;
};

//AND
/**
	In case our expression has multiple variables
	this type of interval express for a continuous space for the expression
	
	For example: 0<x<=5 AND y>3
 */
struct tagCombinedCriteria {
	char objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	Criteria **list;
	unsigned int loggedSize;
	unsigned int size;
};

//OR
struct tagCompositeCriteria {
	char objectType;
	FPCheck fcheck;
	FPGetIntervalF fgetInterval;
	CombinedCriteria **list;
	unsigned int loggedSize;
	unsigned int size;
};

struct tagListCriteria {
	void **list;
	unsigned int loggedSize;
	unsigned int size;
};

struct tagFData {
	DATA_TYPE_FP *data;
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
long l_cast(DATA_TYPE_FP val, DATA_TYPE_FP *fr);
DATA_TYPE_FP parseFloatingPoint(const char *str, int start, int end, int *error);
int contains(int type, const int *aset, int len);
DATA_TYPE_FP logab(DATA_TYPE_FP a, DATA_TYPE_FP b, int *error);
DATA_TYPE_FP doCalculate(DATA_TYPE_FP val1, DATA_TYPE_FP val2, int type, int *error);
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