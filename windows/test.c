#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
	#include <conio.h>
#endif

#include "nlabparser2.h"
#include "nmath.h"

void printError(int col, int code);
int test1(int argc, char *agr[]);
int test2(int argc, char *agr[]);

int main(int argc, char *agr[]){
	//return test1(argc, agr);
	return test2(argc, agr);
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

		case ERROR_SIN:
			break;

		case ERROR_PARENTHESE_MISSING:
			printf("Missing parenthese at %d\n", col);
			break;

		case ERROR_OUT_OF_DOMAIN:
			break;

		case ERROR_SYNTAX:
			break;

		case ERROR_SIN_SQRT:
			break;

		case ERROR_BAD_TOKEN:
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
	resetFunction(&f, "x+3*y^2", "xy", 2, &error);

	error = parseFunction(&f, &l);
	if(error != 0) {
		printError(l, error);
		releaseFunct(&f);
		return error;
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

int test2(int argc, char *agr[]){
	char strF[] = "f(x,y)=x+y";
	int len = 10;
	TokenList lst;
	int i, ret;
	Function *f;

	lst.loggedSize = 10;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;

	/* build the tokens list from the input string */
	parseTokens(f->str, f->len, &lst);
	/* after lexer work, getLexerError() will return -1 if every ok, otherwise it return -1 */
	
	f = parseFunctionExpression(&lst);
	
	for(i = 0; i<lst.size; i++)
		free(lst.list[i]);
	free(lst.list);
	
	return ERROR_LEXER;
}