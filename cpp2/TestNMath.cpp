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

using namespace nmath;

void releaseNMATree(NMASTList **t) {
	int i, j = 0, k = 0;
	char crType;
	
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

void getOperatorChar(int operatorType, char *opCh) {
	switch (operatorType){
	case PLUS:
		(*opCh) = '+';
		return;
	case MINUS:
		(*opCh) = '-';
		return;
	case MULTIPLY:
		(*opCh) = '*';
		return;
	case DIVIDE:
		(*opCh) = '/';
		return;
	case POWER:
		(*opCh) = '^';
		return;
	default:
		(*opCh) = 0;
		return;
	}
}

void toString(const NMAST *t, char *str, int *curpos, int len) {
	double fr;
	long lval;
	int i, l;
	char operatorChar = 0;
	char tmp[16];

	if (t == NULL)
		return;

	switch (t->type){
	case NUMBER:
		lval = l_cast(t->value, &fr);
		if (fr != 0.0){
			l = sprintf(tmp, "%lf", t->value);
		}
		else{
			l = sprintf(tmp, "%ld", lval);
		}

		for (i = 0; i<l; i++) {
			str[(*curpos)] = tmp[i];
			(*curpos)++;
		}
		break;

	case PI_TYPE:
		str[(*curpos)] = 'P';
		str[(*curpos) + 1] = 'I';
		(*curpos) += 2;
		break;

	case E_TYPE:
		str[(*curpos)] = 'e';
		(*curpos)++;
		break;

	case VARIABLE:
	case NAME:
		str[(*curpos)] = t->variable;
		(*curpos)++;
		break;

	case PLUS:
	case MINUS:
	case MULTIPLY:
	case DIVIDE:
	case POWER:
		getOperatorChar(t->type, &operatorChar);

		if ((t->parent != NULL) && ((t->parent)->priority < t->priority)){
			str[(*curpos)] = '(';
			(*curpos)++;
		}


		if (t->left != NULL)
			toString(t->left, str, curpos, len);

		str[(*curpos)] = operatorChar;
		(*curpos)++;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		if ((t->parent != NULL) && ((t->parent)->priority < t->priority)) {
			str[(*curpos)] = ')';
			(*curpos)++;
		}
		break;

	case SIN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';

		//printf("sin(");
		str[(*curpos)] = 's';
		str[(*curpos) + 1] = 'i';
		str[(*curpos) + 2] = 'n';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case ASIN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("asin(");
		str[(*curpos)] = 'a';
		str[(*curpos) + 1] = 's';
		str[(*curpos) + 2] = 'i';
		str[(*curpos) + 3] = 'n';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case COS:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("cos(");
		str[(*curpos)] = 'c';
		str[(*curpos) + 1] = 'o';
		str[(*curpos) + 2] = 's';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case ACOS:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("acos(");
		str[(*curpos)] = 'a';
		str[(*curpos) + 1] = 'c';
		str[(*curpos) + 2] = 'o';
		str[(*curpos) + 3] = 's';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case ATAN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("atan(");
		str[(*curpos)] = 'a';
		str[(*curpos) + 1] = 't';
		str[(*curpos) + 2] = 'a';
		str[(*curpos) + 3] = 'n';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case LN:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("ln(");
		str[(*curpos)] = 'l';
		str[(*curpos) + 1] = 'n';
		str[(*curpos) + 2] = '(';
		(*curpos) += 3;

		if (t->right != NULL)
			toString(t->right, str, curpos, len);

		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case LOG:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("log(");
		str[(*curpos)] = 'l';
		str[(*curpos) + 1] = 'o';
		str[(*curpos) + 2] = 'g';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;
		if (t->left != NULL)
			toString(t->left, str, curpos, len);
		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case SQRT:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("sqrt(");
		str[(*curpos)] = 's';
		str[(*curpos) + 1] = 'q';
		str[(*curpos) + 2] = 'r';
		str[(*curpos) + 3] = 't';
		str[(*curpos) + 4] = '(';
		(*curpos) += 5;
		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case SEC:
		if (t->sign == -1)
			str[(*curpos)++] = '-';
		//printf("sec(");
		str[(*curpos)] = 's';
		str[(*curpos) + 1] = 'e';
		str[(*curpos) + 2] = 'c';
		str[(*curpos) + 3] = '(';
		(*curpos) += 4;
		if (t->right != NULL)
			toString(t->right, str, curpos, len);
		str[(*curpos)] = ')';
		(*curpos)++;
		break;

	case AND:
		str[(*curpos)] = 'A';
		str[(*curpos) + 1] = 'N';
		str[(*curpos) + 2] = 'D';
		(*curpos) += 3;
		break;

	case OR:
		str[(*curpos)] = 'O';
		str[(*curpos) + 1] = 'R';
		(*curpos) += 2;
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[]) {
	NLabLexer lexer(10);
	NLabParser parser;
	NMASTList *domain;
	Criteria *c;
	char outStr[64];
	int start = 0;

	lexer.lexicalAnalysis("a > 10 and a <= 15", 18, 0);
	
	parser.parseDomain(lexer, &start);
	if (parser.getErrorCode() == NMATH_NO_ERROR) {
		domain = parser.domain();
		start = 0;
		//toString(domain->list[0], outStr, &start, 64);
		printNMAST(domain->list[0], 0);
		outStr[start] = '\0';
		puts(outStr);

		c = nmath::buildTree(domain->list[0]);

		releaseNMATree(&domain);

		if (c != NULL)
			delete c;
	}
	else {
		printf("Parsing error with code = %d", parser.getErrorCode());
	}
	

	return 0;
}

