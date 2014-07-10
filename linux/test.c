#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	#include <conio.h>
#endif

#include "common.h"
#include "nlabparser.h"
#include "criteria.h"
#include "nmath.h"

void printError(int col, int code);
int test1(int argc, char *agr[]);
int test2(int argc, char *agr[]);
void testCriteria1(int argc, char *agr[]);
void testCriteria2(int argc, char *agr[]);

int main(int argc, char *agr[]){

	if(argc < 2){
		printf("An mathematics function must be entered.\n");
		printf("Ex: f(x,y) = x + 2 * y\n");
		return 0;
	}
	
	//test1(argc, agr);
	test2(argc, agr);
	//testCriteria1(argc, agr);
	//testCriteria2(argc, agr);
#ifdef DEBUG
	printf("\n[EndOfProgram] Number of dynamic object alive: %d \n", numberOfDynamicObject());
#endif

	return 0;
}

void testCriteria1(int argc, char *agr[]){
	Criteria *ci1, *ci2;
	CombinedCriteria *cc1, *cc2;
	CompositeCriteria *cp1;
	short error;
	int i, j, outlen, chk;
	DATA_TYPE_FP *vals;
	CombinedCriteria *outIntList;
	CompositeCriteria *outDomain;
	
	//Test -1 < x  < 2.1
	ci1 = newCriteria(GT_LT, 'x', -1.0, 2.1, FALSE, FALSE);
	vals = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP));
	vals[0] = parseFloatingPoint(agr[1], 0, strlen(agr[1]), &error);	
	if(error != NO_ERROR)
		return;
	
	chk = ci1->fcheck(ci1, vals, 1);
	if(chk)
		printf("Value %lf is IN (-1.0, 2.1) \n", vals[0]);
	else
		printf("Value %lf is NOT in (-1.0, 2.1) \n", vals[0]);
	
	free(vals);
	vals = NULL;
		
	//Test CombinedInterval  -1 < x  < 2.1 AND 0 < y
	vals = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * 4);
	vals[0] = -0.7f;
	vals[1] = 2.2f;
	vals[2] = -0.9f;
	vals[3] = 1.2f;
	
	outIntList = newCombinedInterval();
	
	ci1->type = GT_LT;
	ci1->variable = 'x';
	ci1->leftVal = -1.0;
	ci1->rightVal = 2.1;
	ci1->flag = ci1->flag & 0xfc;
	
	ci2 = newCriteria(GT_LT, 'y', 0.0, 2.1, FALSE, TRUE);
	
	cc1 = newCombinedInterval();
	cc1->list = (Criteria**)malloc(sizeof(Criteria*) * 2);
	cc1->list[0] = ci1;
	cc1->list[1] = ci2;
	cc1->loggedSize = 2;
	cc1->size = 2;
	
	outlen = -1;
	cc1->fgetInterval(cc1, vals, 2, (void*)outIntList);
	if(outlen >= 0){
		printf("\nOut of space\n");
	}else{
		printf("\nOut(");
		for(i=0; i<outIntList->size; i++){
			printf("(%lf , %lf)", outIntList->list[i]->leftVal, outIntList->list[i]->rightVal);
		}
		printf(")\n");
		
		for(i=0; i<outIntList->size; i++){
			free(outIntList->list[i]);
		}
		free(outIntList->list);
	}
	
	
	free(cc1->list);
	free(cc1);
	free(vals);
	free(outIntList);
	
	//Test CompositeCriteria   x  < -1 OR x > 0
	vals = (DATA_TYPE_FP*)malloc(sizeof(DATA_TYPE_FP) * 2);
	vals[0] = -1.7f;
	vals[1] = 1.3f;
	
	ci1->type = GT_LT;
	ci1->variable = 'x';
	ci1->leftVal = -1.0;
	ci1->rightVal = -1;
	ci1->flag = (ci1->flag | 0x02) & 0xfe;
	
	ci2->type = GT_LT;
	ci2->variable = 'x';
	ci2->leftVal = 0.0;
	ci2->rightVal = 2.1;
	ci1->flag = (ci1->flag & (0xff << 2 )) | 0x01;
	
	cc1 = newCombinedInterval();
	cc1->list = (Criteria**)malloc(sizeof(Criteria*));
	cc1->list[0] = ci1;
	cc1->loggedSize = 1;
	cc1->size = 1;
	
	cc2 = newCombinedInterval();
	cc2->list = (Criteria**)malloc(sizeof(Criteria*));
	cc2->list[0] = ci2;
	cc2->loggedSize = 1;
	cc2->size = 1;
	
	cp1 = newCompositeInterval();
	cp1->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*)*2);
	cp1->list[0] = cc1;
	cp1->list[1] = cc2;
	cp1->loggedSize = 2;
	cp1->size = 2;
	
	outDomain = newCompositeInterval();
	
	cp1->fgetInterval(cp1, vals, 1, (void*)outDomain);
	
	printf("\n");
	for(i=0; i<outDomain->size; i++){
		printf("Out(");
		for(j=0; j<outDomain->list[i]->size; j++)
			printf("(%lf , %lf)", outDomain->list[i]->list[j]->leftVal, outDomain->list[i]->list[j]->rightVal);
		printf(")\n");
	}
		
	for(i=0; i<outDomain->size; i++){
		free(outDomain->list[i]);
	}
	free(outDomain->list);
	
	free(ci1);
	free(ci2);
	free(cc1->list);
	free(cc1);
	free(cc2->list);
	free(cc2);
	free(cp1->list);
	free(cp1);
	free(outDomain);
}

/**
	Test convert from NMAST tree to Criteria
	Date: 6 Jul 2014
	Result: Passed
*/
void testCriteria2(int argc, char *agr[]){
	NMAST *ast;
	OutBuiltCriteria outCriteria;
	Criteria *ci1; 
	//Criteria *ci2;
	//CombinedCriteria *cc1;
	//CombinedCriteria *cc2;
	//CompositeCriteria *cp1;
	//int i;
	//int j; 
	//int error;
	//int outlen;
	//int chk;
	int outType;
	//DATA_TYPE_FP *vals;
	//CombinedCriteria *outIntList;
	//CompositeCriteria *outDomain;
	
	outCriteria.cr = NULL;
	
	//Tes1
	ast = (NMAST*)malloc(sizeof(NMAST));
	ast->type = GT_LT;
	ast->variable = 'x';
	ast->parent = NULL;
	ast->left = (NMAST*)malloc(sizeof(NMAST));
	ast->left->type = NUMBER;
	ast->left->value = -1;
	ast->left->valueType = TYPE_FLOATING_POINT;
	ast->left->left = ast->left->right = NULL;
	ast->left->parent = ast;
	ast->right = (NMAST*)malloc(sizeof(NMAST));
	ast->right->type = NUMBER;
	ast->right->value = 1.5;
	ast->right->valueType = TYPE_FLOATING_POINT;
	ast->right->left = ast->right->right = NULL;
	ast->right->parent = ast;
	
	buildCompositeCriteria(ast, &outCriteria);
	
	if(outCriteria.cr != NULL){
		outType = *((int*)(outCriteria.cr));
		if( outType == SIMPLE_CRITERIA ){
			ci1 = (Criteria*)(outCriteria.cr);
			printf("Got a simple criteria from %lf to %lf\n", ci1->leftVal, ci1->rightVal );
			free(outCriteria.cr);
		} else if( outType == COMBINED_CRITERIA ){
		} else if( outType == COMPOSITE_CRITERIA ){
		}
	}
	clearTree(&ast);
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
	Function *f = (Function*)malloc(sizeof(Function));
	DParam d;
	int error;
	DATA_TYPE_FP vars[] = {4, 1};
	DATA_TYPE_FP ret;
	char dstr[64];
	int l = 0;

	printf("%s\n", agr[1]);
	//resetFunction(&f, agr[1], "xy", 2, &error);
	l = strlen(agr[1]);
	parseFunction(agr[1], l, f);
	if(getErrorCode() != NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(f);
		free(f);
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

int test2(int argc, char *agr[]){
	Function *f;
	int i, l = 0;
	DATA_TYPE_FP bd[]={-1, 1, 0.5, 1.2};
	ListFData *data;

	f = (Function*)malloc(sizeof(Function));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif	
	l = strlen(agr[1]);
	parseFunction(agr[1], l, f);
	if(getErrorCode() != NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(f);
		free(f);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
		return getErrorCode();
	}
	printf("\n");
	data = getSpaces(f, bd, f->valLen * 2, 0.1);
	if(data != NULL){
		for(i=0; i<data->size; i++){
			printf("Mesh %d, row count: %d number of vertex: %d\n", i, data->list[i]->rowCount, data->list[i]->dataSize/data->list[i]->dimension);
			free(data->list[i]->data);
			free(data->list[i]->rowInfo);
			free(data->list[i]);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(3);
#endif
		}
		free(data->list);
		free(data);
#ifdef DEBUG
	descNumberOfDynamicObject(2);
#endif
	}

	releaseFunct(f);
	free(f);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
	return 0;
}
