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
#include "../graphutil.h"

void printError(int col, int code);
int testDerivative(Function *f);
int testGetSpaces(Function *f);
void testReuseFunction(Function *f);
void testCriteria1(int argc, char *agr[]);
void testCriteria2(Function *f);

int main(int argc, char *agr[]){
	Function *f;
	int l;
	if(argc < 2){
		printf("An mathematics function must be entered.\n");
		printf("Ex: f(x,y) = x + 2 * y\n");
		return 0;
	}

	f = (Function*)malloc(sizeof(Function));
	f->str = NULL;
	f->len = 0;
	f->valLen = 0;
	f->prefix = NULL;
	f->domain = NULL;
	f->criterias = NULL;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	printf("%s\n", agr[1]);
	//resetFunction(&f, agr[1], "xy", 2, &error);
	l = strlen(agr[1]);
	parseFunction(agr[1], l, f);
	if(getErrorCode() != NMATH_NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(f);
		free(f);
		return getErrorCode();
	}
	
	//testDerivative(f);
	//testGetSpaces(f);
	//testReuseFunction(f);
	testCriteria2(f);

	releaseFunct(f);
	free(f);
#ifdef DEBUG
	descNumberOfDynamicObject();
	printf("\n[EndOfProgram] Number of dynamic object alive: %d \n", numberOfDynamicObject());
#endif

	return 0;
}

int parseRange(char *str, int len, DATA_TYPE_FP *bd, int *outlen){
	int i=0, j, flag = FALSE;
	short e;
	int k = 0;
	while(i<len){
		if(str[i] >= 48 && str[i] <= 57){
			flag = FALSE;
			for(j=i; j<len; j++){
				if( str[j] <48 || str[j]>57 ){
					if(str[j] == '.'){
						if(flag == TRUE){
							return -1;
						}
						flag = TRUE;
					}else if (str[j] == ' '){
						bd[k++] = parseFloatingPoint(str, i, j-1, &e);
						i = j;
						break;
					}else{
						return -1;
					}
				}
			}
		}
		i++;
	}
	*outlen = k;
	return k;
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
	if(error != NMATH_NO_ERROR)
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
void testCriteria2(Function *f) {
	int i, j;
	char type;
	Criteria *cr;
	CombinedCriteria *cc;
	CompositeCriteria *cp;
	buildCriteria(f);

	if(f->criterias != NULL) {
		for(i=0; i<f->criterias->size; i++) {
			type = *((char*)(f->criterias->list[i]));
			switch(type) {
				case SIMPLE_CRITERIA:
					cr = (Criteria*)f->criterias->list[i];
					switch(cr->type) {
						case LT:
							printf("Simple Criteria type: LT; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;

						case GT:
							printf("Simple Criteria type: GT; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;

						case LTE:
							printf("Simple Criteria type: LTE; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;

						case GTE:
							printf("Simple Criteria type: GTE; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;

						case GT_LT:
							printf("Simple Criteria type: GT_LT; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;

						case GTE_LT:
							printf("Simple Criteria type: GTE_LT; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;

						case GT_LTE:
							printf("Simple Criteria type: GT_LTE; variable: %c; left: %lf; right: %lf\n", cr->variable, cr->leftVal, cr->rightVal);
						break;
					}
					
				break;

				case COMBINED_CRITERIA:
					cc = (CombinedCriteria*)f->criterias->list[i];
					printf("CombinedCriteria size: %d\n", cc->size);
				break;

				case COMPOSITE_CRITERIA:
					cp = (CompositeCriteria*)f->criterias->list[i];
					
					printf("CompositeCriteria size: %d\n", cp->size);
				break;
			}
		}
	}
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

		case ERROR_NOT_A_FUNCTION:
			printf("Bad function notation found at %d\n", col);
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

int testDerivative(Function *f) {
	DParam d;
	int error;
	DATA_TYPE_FP vars[] = {4, 1};
	DATA_TYPE_FP ret;
	char dstr[64];
	int l = 0;
	
	ret = calc(f, vars, 2, &error);
	printf("Ret = %lf \n", ret );
	d.t = f->prefix->list[0];
	d.error = 0;
	d.returnValue = NULL;
	d.variables[0] = 'x';

	derivative(&d);

	toString(d.returnValue, dstr, &l, 64);
	printf("f' = %s\n", dstr);
	clearTree(&(d.returnValue));
	return 0;
}

int testGetSpaces(Function *f){
	int cmd, k, numOfV=0, i, j, vcount;
	DATA_TYPE_FP bd[]={-2, 2, -2, 2};
	DATA_TYPE_FP eps = 0.2f;
	ListFData *data;
	FILE *file;
	short *indice;
	int indiceSize = 0, indiceLoggedSize;
	char strbuff[256];
	
	printf("The range for each variable is [-2, 2]\n");
	printf("The epsilon = %lf \n", eps);
	printf("Do you want to change these values? Press 1 and Enter to change otherwise you will use the default\n");
	scanf("%d", &cmd);
	if(cmd == 1){
		printf("Enter the range for each variable, use space as delimiter: ");
		fflush(stdin);
		gets(strbuff);
		printf("Enter epsilon: ");
		scanf("%lf", &eps);
		parseRange(strbuff, strlen(strbuff), bd, &numOfV);
	}
	
	printf("\n");
	data = getSpaces(f, bd, f->valLen * 2, eps);
	if(data != NULL){
		file = fopen("../data.txt", "w+");
		for(i=0; i<data->size; i++){
			vcount = data->list[i]->dataSize/data->list[i]->dimension;
			printf("Mesh %d, row count: %d number of vertex: %d\n", i, data->list[i]->rowCount, vcount);
			fprintf(file, "number of vertex = %d\n", vcount );
			
			for(j=0; j<vcount; j++){
				fprintf(file, "%lf \t %lf \t %lf\n", data->list[i]->data[j*3], data->list[i]->data[j*3+1], data->list[i]->data[j*3+2]);
			}
			
			indice = buildIndicesForGLLINEs(vcount, data->list[i]->rowInfo, data->list[i]->rowCount, &indiceSize, &indiceLoggedSize);
			if(indice != NULL) {
				cmd = 0;
				for(j=0; j<data->list[i]->rowCount; j++) {
					fprintf(file, "\n ");
					for(k=0; k<data->list[i]->rowInfo[j]; k++)
						fprintf(file, "%d \t ", cmd++);
				}
				
				fprintf(file, "\nIndices length: %d \n", indiceSize);
				k = indiceSize/20;
				for(j=0; j<indiceSize; j++) {
					fprintf(file, "\t %d", indice[j] );
					if(j>0 && (j%k==0))
						fprintf(file, "\n");
				}
				free(indice);
			}
			
			free(data->list[i]->data);
			free(data->list[i]->rowInfo);
			free(data->list[i]);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(3);
#endif
		}
		fclose(file);
		free(data->list);
		free(data);
#ifdef DEBUG
	descNumberOfDynamicObject(2);
#endif
	}
	return 0;
}

void testReuseFunction(Function *f) {
	DATA_TYPE_FP val;
	int error;
	DATA_TYPE_FP var[1] = {10};

	releaseFunct(f);
	parseFunction("f(x)=x + 1/2", 12, f);
	if(getErrorCode() != NMATH_NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(f);
		free(f);
	}

	val = calc(f, var, 1, &error);
	printf("Ret = %lf \n", val );

}
