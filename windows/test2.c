#include <stdlib.h>
#include <stdio.h>
#include "funct.h"
#include "derivative.h"
#include "testlib.h"


#ifdef _WIN32
#include <conio.h>
#endif

int main(int argc, char *agr[]){
	Function f;
	int error = -1;
	/*char s[128]="x^5+3";
	char s[128]="(x+2)^3";
	char s[128]="2*x^4+3*x^2";*/
	char s[128]="cos(x)^3+3*0 + 0+0";
	TNode *df;
	DParam dp;

	initFunct(&f);
	printf("Reseting function with %s\n", s);
	reset(&f, s, &error);
	
	if(error != 0){
		printf("Formular syntax error.\n");
		releaseFunct(&f);
		return 0;
	}
	setVariables(&f, "x", 1);
	printf("Parsing %s\n", s);
	error = parseFunct(&f);
	printf("Parsing done\n", s);
	if(!error){
		reduce(&f, &error);
		if(error == 0){
			printTree(*(f.prefix));
			printf("\n");
			dp.t = *(f.prefix);
			dp.x = 'x';
			df = (TNode *)derivative(&dp);
			
			dp.t = df;
			dp.error = 0;
			reduce_t(&dp);
			
			printTree(df);
			printf("\n");
			clearTree(&df);
		}else{
			printf("Calculating error with code: %d\n", error);
		}
		
	}else
		printf("\nSyntax error with code: %d\n", error);

	releaseFunct(&f);
	return 0;
}
