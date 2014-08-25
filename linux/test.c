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
void testReduce(Function *f);
void testUTF(Function *f);
void testUTFFromFile(Function *f);
void testCalculate(Function *f);

int main(int argc, char *agr[]){
	Function *f;
	/*int l;
	if(argc < 2){
		printf("An mathematics function must be entered.\n");
		printf("Ex: f(x,y) = x + 2 * y\n");
		return 0;
	}*/

	f = (Function*)malloc(sizeof(Function));
	f->str = NULL;
	f->len = 0;
	f->valLen = 0;
	f->prefix = NULL;
	f->domain = NULL;
	f->criterias = NULL;
	/*
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	printf("%s\n", agr[1]);
	l = strlen(agr[1]);
	parseFunction(agr[1], l, f);
	if(getErrorCode() != NMATH_NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
		releaseFunct(f);
		free(f);
		return getErrorCode();
	} else if( (getErrorCode() == NMATH_NO_ERROR) && (f->valLen==0)) {
		printf("This expression is not a function due to variables not determined.\n");
	}
	*/
	//testReduce(f);
	//testDerivative(f);
	//testGetSpaces(f);
	testUTFFromFile(f);
	//testCalculate(f);
	//testReuseFunction(f);
	//testCriteria2(f);

	releaseFunct(f);
	clearPool();
	free(f);
#ifdef DEBUG
	descNumberOfDynamicObject();
	printf("\n[EndOfProgram] Number of dynamic object alive: %d \n", numberOfDynamicObject());
#endif

	return 0;
}

int parseRange(char *str, int len, DATA_TYPE_FP *bd, int *outlen){
	int i=0, j, flag = FALSE;
	int e;
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
	int error;
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

void printSimpleCriteria(const Criteria *cr) {
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
}

void printCombinedCriteria(const CombinedCriteria *cc) {
	int i;
	Criteria *cr;
	if(cc != NULL) {
		printf("CombinedCriteria size: %d\n", cc->size);
		for(i=0; i<cc->size; i++) {
			cr = cc->list[i];
			printSimpleCriteria(cr);
		}
		printf("\n");
	}
}

void printNMAST(NMAST *ast, int level) {
	int i;

	if(ast == NULL) return;

	if(level > 0){
		for(i=0; i<level-1; i++)
			printf("\t");
		printf("|-----");
	}

	switch(ast->type) {
		case AND:
			printf("AND \n");
		break;

		case OR:
			printf("OR \n");
		break;

		case LT:
			printf("LT \n");
		break;

		case GT:
			printf("GT \n");
		break;

		case VARIABLE:
			printf("%c \n", ast->variable);
		break;

		case NUMBER:
			printf("%lf \n", ast->value);
		break;

		case PI_TYPE:
			printf("PI \n");
		break;

		case E_TYPE:
			printf("e \n");
		break;

		case PLUS:
			printf("+ \n");
		break;

		case MINUS:
			printf("- \n");
		break;

		case MULTIPLY:
			printf("* \n");
		break;

		case DIVIDE:
			printf("/ \n");
		break;
	}

	if(ast->left != NULL)
		printNMAST(ast->left, level+1);

	if(ast->right != NULL)
		printNMAST(ast->right, level+1);
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

	if(f->domain != NULL && f->domain->size>0)
		printNMAST(f->domain->list[0], 0);

	buildCriteria(f);

	if(f->criterias != NULL) {
		for(i=0; i<f->criterias->size; i++) {
			type = *((char*)(f->criterias->list[i]));
			switch(type) {
				case SIMPLE_CRITERIA:
					cr = (Criteria*)f->criterias->list[i];
					printSimpleCriteria(cr);
				break;

				case COMBINED_CRITERIA:
					cc = (CombinedCriteria*)f->criterias->list[i];
					printf("CombinedCriteria size: %d\n", cc->size);
					for(j=0; j<cc->size; j++){
						cr = cc->list[j];
						printSimpleCriteria(cr);
					}
				break;

				case COMPOSITE_CRITERIA:
					cp = (CompositeCriteria*)f->criterias->list[i];
					printf("CompositeCriteria size: %d\n", cp->size);
					for(j=0; j<cp->size; j++){
						cc = cp->list[j];
						printCombinedCriteria(cc);
					}
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

		case ERROR_MISSING_FUNCTION_NOTATION:
			printf("This expression is not a function due to variables not determined.\n");
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

void testReduce(Function *f) {
	DParam dp;
	int l = 0;
	char outString[256];

	if(f==NULL || f->prefix == NULL) return;

	dp.t = f->prefix->list[0];
	dp.error = 0;
	reduce_t(&dp);

	/*
		TODO: at this poit the pointer f->prefix->list[0] has been destroyed. Please investigate why
	*/
	toString(dp.t, outString, &l, 256);
	printf("f = %s\n", outString);

	f->prefix->list[0] = dp.t;
}

int testGetSpaces(Function *f){
	int cmd, k, numOfV=0, i, j, vcount;
	DATA_TYPE_FP bd[]={-2, 2, -2, 2};
	DATA_TYPE_FP eps = 0.2f;
	ListFData *data;
	FILE *file;
	int *indice;
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

void testUTF(Function *f) {
	char str[64];
	int len, i, k=0;
	DParam dp;
	TokenList tokens;

	str[0] = 0x31;
	str[1] = 0x2B;
	str[2] = 0xCF;
	str[3] = 0x80;
	len = 4;

	//Lexer
	tokens.loggedSize = len;
	tokens.list = (Token*)malloc(sizeof(Token) * tokens.loggedSize);
	tokens.size = 0;
	/* build the tokens list from the input string */
	lexicalAnalysisUTF8(str, len, &tokens);

	if( getErrorCode() == NMATH_NO_ERROR ) {
		printf("lexical analysis successfully, number of token: %d\n", tokens.size);

		parseExpression(&tokens, &k, f);

		if( getErrorCode() == NMATH_NO_ERROR ) {
			dp.error = NMATH_NO_ERROR;
			dp.t = f->prefix->list[0];
			reduce_t(&dp);
			if(dp.error == NMATH_NO_ERROR) {
				k = 0;
				printNMAST(dp.t, 0);
				printf("\n");
				toString(dp.t, str, &k, 64);
				f->prefix->list[0] = dp.t;
				printf("Calculating result: %s\n", str);
			}
		}

		//release token list
			
		for(i = 0; i<tokens.size; i++){
			printf("%X ", tokens.list[i].type);
		}

		printf("\n");
		free(tokens.list);
	}

	if(getErrorCode() != NMATH_NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
	}
}

void testUTFFromFile(Function *f) {
	FILE *file;
	char str[64];
	int len, size, i, k=0;
	DParam dp;
	TokenList tokens;

	file = fopen("/cygdrive/d/data.dat", "rb");
	if( file != NULL ) {
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		len = fread(str, 1, size, file);
		fclose(file);

		printf("Number of bytes read: %d\n", len);
		for(i = 0; i<len; i++){
			printf("0x%X(%d) ", str[i],str[i]);
		}
		printf("\n");

		//Lexer
		tokens.loggedSize = len;
		tokens.list = (Token*)malloc(sizeof(Token) * tokens.loggedSize);
		tokens.size = 0;
		/* build the tokens list from the input string */
		lexicalAnalysisUTF8(str, len, &tokens);

		if( getErrorCode() == NMATH_NO_ERROR ) {
			printf("lexical analysis successfully, number of token: %d\n", tokens.size);

			parseExpression(&tokens, &k, f);

			if( getErrorCode() == NMATH_NO_ERROR ) {
				dp.error = NMATH_NO_ERROR;
				dp.t = f->prefix->list[0];
				reduce_t(&dp);
				if(dp.error == NMATH_NO_ERROR) {
					k = 0;
					printNMAST(dp.t, 0);
					printf("\n");
					toString(dp.t, str, &k, 64);
					str[k] = 0;
					f->prefix->list[0] = dp.t;
					printf("Calculating result: %s\n", str);
				}
			}

			//release token list
			
			for(i = 0; i<tokens.size; i++){
				printf("%X ", tokens.list[i].type);
			}
			printf("\n");
			free(tokens.list);
		}

		if(getErrorCode() != NMATH_NO_ERROR) {
			printError(getErrorColumn(), getErrorCode());
		}
	}
}

void testCalculate(Function *f) {
	char str[64];
	int len=0, i, k=0;
	DParam dp;
	TokenList tokens;

	printf("Number of bytes read: %d\n", len);
	for(i = 0; i<len; i++){
		printf("0x%X(%d) ", str[i],str[i]);
	}
	printf("\n");

	//Lexer
	tokens.loggedSize = len;
	tokens.list = (Token*)malloc(sizeof(Token) * tokens.loggedSize);
	tokens.size = 0;
	/* build the tokens list from the input string */
	//lexicalAnalysis(exp, expLen, &tokens);
	lexicalAnalysisUTF8(str, len, &tokens);

	if( getErrorCode() == NMATH_NO_ERROR ) {
		printf("lexical analysis successfully, number of token: %d\n", tokens.size);

		parseExpression(&tokens, &k, f);

		if( getErrorCode() == NMATH_NO_ERROR ) {
			dp.error = NMATH_NO_ERROR;
			dp.t = f->prefix->list[0];
			reduce_t(&dp);
			if(dp.error == NMATH_NO_ERROR) {
				k = 0;
				printNMAST(dp.t, 0);
				printf("\n");
				toString(dp.t, str, &k, 64);
				f->prefix->list[0] = dp.t;
				printf("Calculating result: %s\n", str);
			}
		}

		//release token list
			
		for(i = 0; i<tokens.size; i++){
			printf("%X ", tokens.list[i].type);
		}
		printf("\n");
		free(tokens.list);
	}

	if(getErrorCode() != NMATH_NO_ERROR) {
		printError(getErrorColumn(), getErrorCode());
	}
}
