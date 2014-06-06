#ifndef _NLABLEXER_H
#define _NLABLEXER_H

#include "common.h"

void parseTokens(const char *inStr, int length, TokenList *tokens);
int isLogicOperator(char c);
int isDigit(char c);
Token* checkCommaSemi(char c, int *idx);
Token* checkParenthesePrackets(char c, int *idx);
int isNumericOperatorOREQ(char c);
int parserLogicOperator(const char *inStr, int length, int i, char charAtI, int k, char charAtK);
Token* checkNumericOperator(const char *inStr, int length, int *idx);
Token* parsSubtractSign(const char *inStr, int length, int *idx);
int isFunctionName(int index, const char *inStr, int l);
int isVariable(int index, const char *inStr, int length);

#endif