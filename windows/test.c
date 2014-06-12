#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "nmath.h"

int main(int argc, char *agr[]){
	Function f;
	DParam d;
	int error;
	double vars[] = {4};
	double ret;
	char dstr[32];
	int l = 0;

	initFunct(&f);
	resetFunction(&f, "x+5+x^2", "x", 1, &error);
	parseFunction(&f, &error);
	ret = calc(&f, vars, 1, &error);
	printf("Ret = %lf", ret );
	d.t = *(f.prefix);
	d.error = 0;
	d.returnValue = NULL;
	d.x = 'x';
	derivative(&d);

	toString(d.returnValue, dstr, &l, 32);
	clearTree(d.returnValue);

	releaseFunct(&f);

	return 0;
}
