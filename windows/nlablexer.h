#ifndef _NLABLEXER_H
#define _NLABLEXER_H

struct tagToken{
	int type;
	char text[20];
	int testLength;
	/* This is used for operators & functions */
	char priority;
}Token;


void parseToken(char *, int, Token **);
int isLogicOperator(char c);
int isDigit(char c);
int isFunctionName(int index, char *inputString, int l);
int isVariable(int index, char *inputString, int length);

#endif