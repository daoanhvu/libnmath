#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "nmath.h"

int main(int argc, char *agr[]){
	Function f;
	int error;
	double vars[] = {4};
	double ret;

	initFunct(&f);
	resetFunction(&f, "x+5+2^2", "x", 1, &error);
	parseFunction(&f, &error);
	ret = calc(&f, vars, 1, &error);

	printf("Ret = %lf", ret );

	releaseFunct(&f);

	return 0;
}
