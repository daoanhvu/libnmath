#ifndef _TESTLIB
#define _TESTLIB

#include "funct.h"
#include <stdio.h>

void printTree(TNode *t){
	double fr;
	long lval;
	
	if(t==NULL)
		return;
		
	switch(t->function){
		case F_COE:
			lval = l_cast(t->value, &fr);
			if(fr != 0.0)
				printf("%lf", t->value);
			else
				printf("%ld", lval);
		break;
		
		case F_VAR:
			printf("%c", t->chr);
		break;
		
		case F_OPT:
			/*if( (t->parent != NULL) && (t->parent)->priority < t->priority)*/
				printf("(");
			if(t->left != NULL)
				printTree(t->left);
			printf("%c", t->chr);
			if(t->right != NULL)
				printTree(t->right);
			/*if( (t->parent != NULL) && (t->parent)->priority < t->priority)*/
				printf(")");
		break;
		
		case F_FUNCT:
			if(t->sign == -1)
				printf("-");
				
			switch(t->chr){
				case SIN:
					printf("sin(");
					if(t->right != NULL)
						printTree(t->right);
					printf(")");
				break;
				
				case ASIN:
					printf(" asin ");
				break;
				
				case COS:
					printf("cos(");
					if(t->right != NULL)
						printTree(t->right);
					printf(")");
				break;
				
				case ACOS:
					printf(" acos ");
				break;
				
				case LN:
					printf(" ln(");
					if(t->right != NULL)
						printTree(t->right);
					printf(")");
				break;
				
				case LOG:
					printf(" log(");
					if(t->left != NULL)
						printTree(t->left);
					if(t->right != NULL)
						printTree(t->right);
					printf(")");
				break;
				
				case POW:
					/*printf("(");*/
					if(t->left != NULL)
						printTree(t->left);
					printf("^");
					if(t->right != NULL)
						printTree(t->right);
					/*printf(")");*/
				break;
			}
		break;
	}
}

void toString(TNode *t, char *str, int *curpos, int len){
	double fr;
	long lval;
	char tmp[16];
	int tmplen = 0;
	
	if(t==NULL)
		return;
		
	switch(t->function){
		case F_COE:
			lval = l_cast(t->value, &fr);
			if(fr != 0.0){
				tmplen = sprintf(tmp,"%lf", t->value);
				memcpy( (str + (*curpos)), tmp, tmplen);
				(*curpos) += tmplen;
			}
			else{
				tmplen = sprintf(tmp, "%ld", lval);
				memcpy( (str + *curpos), tmp, tmplen);
				(*curpos) += tmplen;
			}
		break;
		
		case F_VAR:
			str[*curpos] = t->chr;
			(*curpos)++;
		break;
		
		case F_OPT:
			/*if( (t->parent != NULL) && (t->parent)->priority < t->priority)*/
			str[*curpos] = '(';
			(*curpos)++;
			if(t->left != NULL)
				toString(t->left, str, curpos, len);
			str[*curpos] = t->chr;
			(*curpos)++;
			if(t->right != NULL)
				toString(t->right, str, curpos, len);
			/*if( (t->parent != NULL) && (t->parent)->priority < t->priority)*/
			str[*curpos] = ')';
			(*curpos)++;
		break;
		
		case F_FUNCT:
			if(t->sign == -1){
				str[*curpos] = '-';
				(*curpos)++;
			}
				
			switch(t->chr){
				case SIN:
					str[*curpos] = 's';
					str[*curpos + 1] = 'i';
					str[*curpos + 2] = 'n';
					str[*curpos + 3] = '(';
					(*curpos) += 3;
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					str[*curpos] = ')';
					(*curpos)++;
				break;
				
				case ASIN:
					str[*curpos] = 'a';
					str[*curpos + 1] = 's';
					str[*curpos + 2] = 'i';
					str[*curpos + 3] = 'n';
					str[*curpos + 4] = '(';
					(*curpos) += 4;
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					str[*curpos] = ')';
					(*curpos)++;
				break;
				
				case COS:
					str[*curpos] = 'c';
					str[*curpos + 1] = 'o';
					str[*curpos + 2] = 's';
					str[*curpos + 3] = '(';
					(*curpos) += 3;
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					str[*curpos] = ')';
					(*curpos)++;
				break;
				
				case ACOS:
					str[*curpos] = 'a';
					str[*curpos + 1] = 'c';
					str[*curpos + 2] = 'o';
					str[*curpos + 3] = 's';
					str[*curpos + 4] = '(';
					(*curpos) += 4;
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					str[*curpos] = ')';
					(*curpos)++;
				break;
				
				case LN:
					str[*curpos] = 'l';
					str[*curpos + 1] = 'n';
					str[*curpos + 2] = '(';
					(*curpos) += 2;
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					str[*curpos] = ')';
					(*curpos)++;
				break;
				
				case LOG:
					str[*curpos] = 'l';
					str[*curpos + 1] = 'o';
					str[*curpos + 2] = 'g';
					str[*curpos + 3] = '(';
					(*curpos) += 3;
					if(t->left != NULL)
						toString(t->left, str, curpos, len);
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					str[*curpos] = ')';
					(*curpos)++;
				break;
				
				case POW:
					/*printf("(");*/
					if(t->left != NULL)
						toString(t->left, str, curpos, len);
					str[*curpos] = '^';
					(*curpos)++;
					if(t->right != NULL)
						toString(t->right, str, curpos, len);
					/*printf(")");*/
				break;
			}
		break;
	}
}

#endif
