// TestNMath.cpp : Defines the entry point for the console application.
//

#include <SDKDDKVer.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <nlablexer.h>
#include <nlabparser.h>
#include <criteria.h>
#include <SimpleCriteria.h>
#include <CompositeCriteria.h>

using namespace nmath;

void releaseNMATree(NMASTList **t) {
	int i, j = 0, k = 0;
	
	if (t == NULL) return;

	if (t != NULL){
		for (i = 0; i<(*t)->size; i++){
			clearTree(&((*t)->list[i]));
		}
		if ((*t)->list != NULL){
			free((*t)->list);
		}
		(*t)->size = 0;
		(*t)->loggedSize = 0;
		free(*t);
		t = NULL;
	}
}

void printCriteria(Criteria *c) {
	int i, n;

	if (c == NULL) return;
	printf("\n");
	switch (c->getCClassType()){
		case SIMPLE:
			printf("(%lf,%lf)", ((SimpleCriteria*)c)->getLeftValue(), ((SimpleCriteria*)c)->getRightValue());
			break;
		case COMPOSITE:
			n = ((CompositeCriteria*)c)->size();
			for (i = 0; i < n; i++)
				printCriteria((*((CompositeCriteria*)c))[i]);
			break;
	}
}

void printNMAST(NMAST *ast, int level) {
	int i;

	if (ast == NULL) return;

	if (level > 0){
		for (i = 0; i<level - 1; i++)
			printf("\t");
		printf("|-----");
	}

	switch (ast->type) {
	case AND:
		printf("AND \n");
		break;

	case OR:
		printf("OR \n");
		break;

	case LT:
		printf("< \n");
		break;

	case LTE:
		printf("<= \n");
		break;

	case GT:
		printf("GT \n");
		break;

	case NAME:
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

	default:
		printf("(type=%d) \n", ast->type);
	}

	if (ast->left != NULL)
		printNMAST(ast->left, level + 1);

	if (ast->right != NULL)
		printNMAST(ast->right, level + 1);
}

int _tmain(int argc, _TCHAR* argv[]) {
	NLabLexer lexer(10);
	NLabParser parser;
	NMASTList *domain;
	Criteria *c, *o;
	char outStr[64];
	int start = 0;
	double value[] = {3, 12};

	lexer.lexicalAnalysis("a > 10 and a <= 15", 18, 0);
	
	parser.parseDomain(lexer, &start);
	if (parser.getErrorCode() == NMATH_NO_ERROR) {
		domain = parser.domain();
		start = 0;
		printNMAST(domain->list[0], 0);
		outStr[start] = '\0';
		puts(outStr);

		c = nmath::buildCriteria(domain->list[0]);

		o = c->getInterval(value, "a", 1);

		printCriteria(c);
		printCriteria(o);
		releaseNMATree(&domain);

		if (c != NULL)
			delete c;

		if (o != NULL)
			delete o;
	} else {
		printf("Parsing error with code = %d", parser.getErrorCode());
	}
	
	return 0;
}
