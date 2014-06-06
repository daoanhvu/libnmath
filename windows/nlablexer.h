#ifndef _NLABLEXER_H
#define _NLABLEXER_H

#include "common.h"

void parseToken(char *, int, Token **);
int isLogicOperator(char c);
int isDigit(char c);
int isFunctionName(int index, char *inputString, int l);
int isVariable(int index, char *inputString, int length);

#endif