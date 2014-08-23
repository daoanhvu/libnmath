#ifndef _NLABLEXER_H
#define _NLABLEXER_H

#include "common.h"

void lexicalAnalysisUTF8(const char *inStr, int length, TokenList *tokens);
int getLexerError();
int isLogicOperator(char c);
int isDigit(char c);

/**
	Check comma or semicolon.
	Return TRUE if it's a semiconlon or comma
	otherwise return FALSE
	In case it returns TRUE, type will hold the actual type of the token
*/
int checkCommaSemi(char c, int *type);
/**
	Check Parenthese or prackets

	Return TRUE if it's a Parenthese or prackets
	otherwise return FALSE

	In case it returns TRUE, type will hold the actual type of the token
*/
int checkParenthesePrackets(char c, int *type);

int isNumericOperatorOREQ(char c);
int parserLogicOperator(const char *inStr, int length, int idx, int *type, int *outlen);
int checkNumericOperator(const TokenList *tokens, const char *inStr, int length, int idx, int *type, int *textLength);
int parseSubtractSign(const TokenList *tokens, const char *inStr, int length, int idx, int *type, int *outlen);
int isFunctionName(const char *inputString, int l, int index, int *outType, int *outlen);
int isAName(const char *inputString, int length, int index);

#endif