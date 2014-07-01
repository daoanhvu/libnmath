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
	
	test1(argc, agr);
#ifdef DEBUG
	printf("\n[EndOfProgram] Number of dynamic object alive: %d \n", numberOfDynamicObject());
#endif

	return 0;
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
	Function *f;
	DParam d;
	int error;
	double vars[] = {4, 1};
	double ret;
	char dstr[64];
	int l = 0;

	printf("%s\n", agr[1]);
	//resetFunction(&f, agr[1], "xy", 2, &error);
	l = strlen(agr[1]);
	f = parseFunction(agr[1], l);
	if(getErrorCode() != NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(f);
		return getErrorCode();
	}
	l = 0;
	ret = calc(f, vars, 2, &error);
	printf("Ret = %lf \n", ret );
	d.t = f->prefix->list[0];
	d.error = 0;
	d.returnValue = NULL;
	d.x = 'x';

//#ifdef __unix
	d.returnValue = derivative(&d);
//#else
//	derivative(&d);
//#endif

	toString(d.returnValue, dstr, &l, 64);
	printf("f' = %s\n", dstr);
	clearTree(&(d.returnValue));

	releaseFunct(f);
	free(f);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
	return 0;
}
