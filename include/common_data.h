#ifndef _COMMON_DATA_H
#define _COMMON_DATA_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>

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
#define ABS				0x00010014
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

#define TYPE_FLOATING_POINT		0
#define TYPE_FRACTION			1

#define COE_VAL_PRIORITY 		0x00
#define PRIORITY_0 				0x00
#define PLUS_MINUS_PRIORITY 	0x01
#define MUL_DIV_PRIORITY 		0x02
#define FUNCTION_PRIORITY 		0x03

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
#define MAX_VAR_COUNT 8
#define PI			3.14159265358979323846
#define E			2.718281828

#define D2R(x) x*(3.141592f/180.0f)


namespace nmath {

    const int FUNCTIONS[] = {SIN, COS, TAN, COTAN, ASIN, ACOS, ATAN, LOG, LN, SQRT};
    const int FUNCTION_COUNT = 10;
    const int OPERATORS[] = {PLUS,MINUS,MULTIPLY,DIVIDE,POWER};
    const int OPERATOR_COUNT = 5;
    const int COMPARING_OPERATORS[] = {LT,LTE,EQ,GT,GTE};
    const int COMPARING_OPERATORS_COUNT = 5;

	struct Token {
		int type;
		char text[MAXTEXTLEN];
		int column;
		unsigned char textLength;
		/* This is used for operators & functions */
		int priority;
	};

	template <typename VT>
	struct NMAST {
		int type;
		int priority;
		VT value;
		std::string text;
		/* this flag is just used for function cause the function cannot express its sign itself */
		/* MUST = 1 by default */
		int sign;
		NMAST<VT> *parent;
		NMAST<VT> *left;
		NMAST<VT> *right;
		char level;
	};

	/** ================================================================================================ */

    /**
     * This structure is used to hold all calculated data of a function
     */
    template <typename T>
	struct FData {
		T *data;
		unsigned int dataSize;
		unsigned int loggedSize;
		char dimension;
		/**
		 * rowInfo holds the number of element on each row
		 * rowCount is the number of row
		*/
		int *rowInfo;
		unsigned int rowCount;
		unsigned int loggedRowCount;
	};

    template <typename T>
	struct ListFData {
		FData<T> **list;
		unsigned long loggedSize;
		unsigned long size;
	};

	/**
	 * This structure is used for function evaluation and derivative calculation
	 *
	 */
	template <typename T>
	struct DParam {
		NMAST<T> *t;
		std::string variables[4];
		int varCount;
		int error;
		T values[MAX_VAR_COUNT];
		T retv;
		NMAST<T> *returnValue;
	};
} //end namespace nmath
#endif
