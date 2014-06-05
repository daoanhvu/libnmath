#include <stdlib.h>
#include <stdio.h>
#include "funct.h"
#include "derivative.h"
#include "testlib.h"
#include <conio.h>

int main(int argc, char *agr[]){
	Function f;
	int error = -1;
	int idxError;
	/*char s[128]="x^5+3";
	char s[128]="(x+2)^3";
	char s[128]="2*x^4+3*x^2";
	char s[128]="cos(x)^3+3*0 + 0+0";*/
	char s[128]="x^2 + y";
	TNode *df;
	DParam dp;
	RParam rp;
	double var[2] = {2, 1.5};
	double ret;

	initFunct(&f);
	printf("Reseting function with %s\n", s);
	reset(&f, s, &error);
	
	if(error != 0){
		printf("Formular syntax error.\n");
		releaseFunct(&f);
		return 0;
	}
	setVariables(&f, "xy", 2);
	printf("Parsing %s\n", s);
	error = parseFunct(&f, &idxError);
	printf("Parsing done\n");
	if(!error){
		reduce(&f, &error);
		printf("Reducing done\n");
		if(error == 0){
			printTree(*(f.prefix));
			ret = calc(&f, var, 2, &error);
			printf("\nf(%lf) = %lf\n", var[0], ret);
			/*
			printf("\n");
			dp.t = *(f.prefix);
			dp.x = 'x';
			derivative(&dp);
			dp.error = 0;

			dp.t = dp.return_value;
			reduce_t(&dp);
			
			printTree(dp.t);
			printf("\n");
			clearTree(&(dp.t));
			*/
		}else{
			printf("Calculating error with code: %d\n", error);
		}
		
	}else
		printf("\nSyntax error with code: %d\n", error);

	releaseFunct(&f);
	return 0;
}
