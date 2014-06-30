#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	#include <conio.h>
#endif

#include "common.h"
#include "nlabparser2.h"
#include "nmath.h"

void printError(int col, int code);
int test1(int argc, char *agr[]);
//int test2(int argc, char *agr[]);

int main(int argc, char *agr[]){

	if(argc < 2){
		printf("An mathematics function must be entered.\n");
		printf("Ex: f(x,y) = x + 2 * y\n");
		return 0;
	}
	
	return test1(argc, agr);
	//return test2(argc, agr);
}

void printError(int col, int code){
	switch(code){
		case ERROR_DIV_BY_ZERO:
			break;

		case ERROR_LOG:
			break;

		case ERROR_OPERAND_MISSING:
			break;

		case ERROR_PARSE:
			break;

		case ERROR_TOO_MANY_FLOATING_POINT:
			printf("Too many floating point at %d\n", col);
			break;

		case ERROR_PARENTHESE_MISSING:
			printf("Missing parenthese at %d\n", col);
			break;
			
		case ERROR_TOO_MANY_PARENTHESE:
			printf("Too many parenthese at %d\n", col);
			break;

		case ERROR_OUT_OF_DOMAIN:
			break;

		case ERROR_SYNTAX:
			break;

		case ERROR_NOT_AN_EXPRESSION:
			printf("Bad expression found at %d\n", col);
			break;

		case ERROR_BAD_TOKEN:
			printf("A bad token found at %d\n", col);
			break;

		case ERROR_LEXER:
			break;

		case ERROR_PARSING_NUMBER:
			break;
	}
}

int test1(int argc, char *agr[]){
	Function f;
	DParam d;
	int error;
	double vars[] = {4, 1};
	double ret;
	char dstr[32];
	int l = 0;

	initFunct(&f);
	printf("x+3*y^2\n");
	resetFunction(&f, agr[1], "xy", 2, &error);

	parseFunction(&f);
	if(getErrorCode() != NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(&f);
		return getErrorCode();
	}
	l = 0;
	ret = calc(&f, vars, 2, &error);
	printf("Ret = %lf", ret );
	d.t = *(f.prefix);
	d.error = 0;
	d.returnValue = NULL;
	d.x = 'x';
	derivative(&d);

	toString(d.returnValue, dstr, &l, 32);
	clearTree(&(d.returnValue));

	releaseFunct(&f);
	return 0;
}
/*
int test2(int argc, char *agr[]){
	char strF[128];
	char s[128];
	int len = 10;
	TokenList lst;
	int i, ret=0;
	Function *f;

	lst.loggedSize = 10;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;
	
	len = strlen(agr[1]);
	for(i=0; i<len; i++)
		strF[i] = agr[1][i];
	strF[i] = '\0';

	// build the tokens list from the input string
	parseTokens(strF, len, &lst);
	// after lexer work, getLexerError() will return -1 if every ok, otherwise it return -1
	
	if(getErrorCode() != NO_ERROR){
		printError(getErrorColumn(), getErrorCode());
		for(i = 0; i<lst.size; i++)
			free(lst.list[i]);
		free(lst.list);
		return ERROR_LEXER;
	}
	
	//Test token list 
	printf("\n");
	for(i = 0; i<lst.size; i++){
		printf(" %s", lst.list[i]->text);
	}
	printf("\n");
	
	f = parseFunctionExpression(&lst);
	
	for(i = 0; i<lst.size; i++)
		free(lst.list[i]);
	free(lst.list);
	
	if(getErrorCode() != NO_ERROR){
		printError(getErrorColumn(), getErrorCode());
		if(f != NULL)
			releaseFunct(f);
		return ERROR_PARSE;
	}
	
	toString(f->prefix[0], s, &ret, 128);
	s[ret] = '\0';
	
	printf("f = %s", s);
	
	if(f != NULL)
		releaseFunct(f);
	
	return NO_ERROR;
}*/