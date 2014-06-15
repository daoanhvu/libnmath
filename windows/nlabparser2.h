#ifndef _NLABPARSER2_H
#define _NLABPARSER2_H

#include "common.h"

struct tagNMAST2{
	int type;

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
	struct tagNMAST2 *parent;
	struct tagNMAST2 **children;
}NMAST2;

#endif