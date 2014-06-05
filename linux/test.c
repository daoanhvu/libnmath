#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "funct.h"
#include "derivative.h"
#include "testlib.h"


#ifdef _WIN32
#include <conio.h>
#endif

void printHelp();
void calculationF(char strF[], char vars[], int varlen);
void derivativeF(char strF[], char vars[], int varlen);
void reduceF(char strF[], char vars[], int varlen);
void printStringF(char strF[], char vars[], int varlen);

int main(int argc, char *args[]){
	int cmpv = 1;
	int varlen = 0;
	
	if(argc <= 3 ){
		printHelp();
		return 0;
	}

	varlen = strlen(args[3]);	
	cmpv = strcmp(args[1], "-c");
	if(cmpv == 0){
		calculationF(args[2], args[3], varlen);
		return 0;
	}
	
	cmpv = strcmp(args[1], "-d");
	if(cmpv == 0){
		derivativeF(args[2], args[3], varlen);
		return 0;
	}
	
	cmpv = strcmp(args[1], "-r");
	if(cmpv == 0){
		reduceF(args[2], args[3], varlen);
		return 0;
	}
	
	cmpv = strcmp(args[1], "-p");
	if(cmpv == 0){
		printStringF(args[2], args[3], varlen);
		return 0;
	}
	
	printf("Syntax error - quit\n");
	return 0;
}

void printHelp(){
	printf("nmath <action> <function> <variables>\n");
	printf("  action: \n");
	printf("  \t-c - Calculate the value of formula\n");
	printf("  \t-d - Calculate the derivative of function\n");
	printf("  \t-d - Reduce formula/function\n");
	printf("  \t-p - print thru a string formula/function\n");
}

void reduceF(char strF[], char vars[], int varlen){
	Function f;
	int error = -1;
	int eIdx = -1;

	initFunct(&f);	
	reset(&f, strF, &error);
	if(error != 0){
		printf("Formular syntax error.\n");
		releaseFunct(&f);
		return;
	}
	
	setVariables(&f, vars, varlen);
	error = parseFunct(&f, &eIdx);
	if(error){
		printf("\nSyntax error with code: %d at pos: %d\n", error, eIdx);
		releaseFunct(&f);
		return;
	}
	
	reduce(&f, &error);
	if(error != 0){
		printf("Calculating error with code: %d\n", error);
		releaseFunct(&f);
		return;
	}
	
	printTree(*(f.prefix));
	printf("\n");
	releaseFunct(&f);
}

void calculationF(char strF[], char vars[], int varlen){
	Function f;
	int i, error = -1;
	int eIdx = -1;
	double tmp, ret = 0;
	double values[4];

	initFunct(&f);	
	reset(&f, strF, &error);
	if(error != 0){
		printf("Formular syntax error.\n");
		releaseFunct(&f);
		return;
	}
	
	setVariables(&f, vars, varlen);
	error = parseFunct(&f, &eIdx);
	if(error){
		printf("\nSyntax error with code: %d at %d\n", error, eIdx);
		releaseFunct(&f);
		return;
	}
	
	reduce(&f, &error);
	if(error != 0){
		printf("Calculating error with code: %d\n", error);
		releaseFunct(&f);
		return;
	}
	
	printTree(*(f.prefix));
	printf("\n");
	printf("Please provide variable values: \n");
	for(i=0; i<varlen; i++){
		printf("%c = ", vars[i]);
		scanf("%lf", &tmp);
		values[i] = tmp;
	}
	ret = calc(&f, values, varlen, &error);
	if(error != 0){
		printf("Error occur while calculating (ErrorCode=%d)\n", error);
		releaseFunct(&f);
		return;
	}
	
	printf("f(%s)=%lf\n", vars, ret);
	releaseFunct(&f);
}

void derivativeF(char strF[], char vars[], int varlen){
	Function f;
	int error = -1;
	int eIdx = -1;
	TNode *df;
	DParam dp;
	
	initFunct(&f);	
	reset(&f, strF, &error);
	if(error != 0){
		printf("Formular syntax error.\n");
		releaseFunct(&f);
		return;
	}
	setVariables(&f, vars, varlen);
	error = parseFunct(&f, &eIdx);
	
	if(error){
		printf("\nSyntax error with code: %d at position %d \n", error, eIdx);
		releaseFunct(&f);
		return;
	}
	
	reduce(&f, &error);
	if(error != 0){
		printf("Calculating error with code: %d\n", error);
		releaseFunct(&f);
		return;
	}
	printf("f(%s) = ", vars);
	printTree(*(f.prefix));
	printf("\n");
	dp.t = *(f.prefix);
	dp.x = 'x';
	df = (TNode *)derivative(&dp);
	
	dp.t = df;
	dp.error = 0;
	reduce_t(&dp);
	printf("f' = ");
	printTree(df);
	printf("\n");
	clearTree(&df);
	releaseFunct(&f);
}


void printStringF(char strF[], char vars[], int varlen){
	Function f;
	int error = -1;
	int eIdx = -1;
	int curpos = 0;
	char str[256];
	
	
	initFunct(&f);	
	reset(&f, strF, &error);
	if(error != 0){
		printf("Formular syntax error.\n");
		releaseFunct(&f);
		return;
	}
	setVariables(&f, vars, varlen);
	error = parseFunct(&f, &eIdx);
	
	if(error){
		printf("\nSyntax error with code: %d at position %d \n", error, eIdx);
		releaseFunct(&f);
		return;
	}
	
	reduce(&f, &error);
	if(error != 0){
		printf("Calculating error with code: %d\n", error);
		releaseFunct(&f);
		return;
	}
	printf("f(%s) = ", vars);
	toString(*(f.prefix), str, &curpos, 256);
	str[curpos] = 0;
	curpos++;
	puts(str);
	printf("\n");
	releaseFunct(&f);
}
