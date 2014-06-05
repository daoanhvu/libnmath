#ifndef _FUNCT_H_
#define _FUNCT_H_

#include "nmbase.h"

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
#define NO_PRIORITY 0x00
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

#define PI 3.14159265358979323846 /* 3.1415926535897932384626433832795 */
#define E 2.718281828

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagStack StackItem;
typedef struct tagTNode TNode;

typedef struct tagFunct{
	char *str;
	int len;

	char variable[3];
	char valLen;

	TNode **prefix;
	int prefixLen;
	int prefixAllocLen;
} Function;

struct tagStack{
	char function;
	int chr;
	char priority;
};

struct tagTNode{
	/* 0:coefficient; 1:variable; 2: functions; 3:Operators; 4: ( or ) */
	char function;
	
	/* This is used for operators & functions */
	char priority;
	
	/* SEE: flags section */
	int chr;
	
	/*
	 TYPE_FLOATING_POINT OR TYPE_FRACTION
	 * */
	char valueType;
	
	double value;
	Fraction frValue;
	
	/* this flag is just used for FUNCTIONs and VARIABLEs cause they cannot express its sign itself */
	/* MUST = 1 by default */
	int sign;
	
	struct tagTNode *parent;
	struct tagTNode *left;
	struct tagTNode *right;
};

typedef struct tagDParam{
	TNode *t;
	char x;
	int error;
}DParam;

typedef struct tagRParam{
	TNode *t;
	char *variables;
	double *values;
	double retv;
	int error;
}RParam;

void initFunct(Function *);
void reset(Function *f, const char *str, int *error);
void setVariables(Function *f, char variable[], int l);

/**
 * This function analysic and parse a function from string into prefix tree (stack)
 * @params:
 * 		f: function object to be parsed
 * 		idxE: in case of error occurred, this param will hold the position that the error has occurred
 * @return:
 * 		0 if ererything is OK, otherwise a number less than zero will be returned, in that case we
 * 		will take case of idxE value.
 * 		
 * */
int parseFunct(Function *f, int *idxE);
void clearTree(TNode **prf);
double calc(Function *f, double *values, int numOfValue, int *);
int reduce(Function *f, int *);
void* reduce_t(void *);
void* calc_t(void *);
void nodeToString(TNode *t, char *str, int len, int *newlen);
void releaseFunct(Function *);

#ifdef __cplusplus
}
#endif

#endif
