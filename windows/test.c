#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "nmath.h"

int main(int argc, char *agr[]){
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
		switch(error){
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
			printf("Missing parenthese at %d\n", l);
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
