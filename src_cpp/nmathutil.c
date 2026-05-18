#include "nmathutil.h"

void generateLevel(const NMAST *t, NMAST **tokens, int *size, char level) {
	char savedLevel;
	
	if(t==NULL)
		return;
		
	t->level = level;
	switch(t->type){
		/*
		case NUMBER:
		case PI_TYPE:
		case E_TYPE:
		case VARIABLE:
		case NAME:
			tokens[size] = t;
			size++;
		break;
		*/
		
		case PLUS:
		case MINUS:
		case MULTIPLY:
		case DIVIDE:
			generateLevel(t->left, tokens, size, level);
			tokens[*size] = t;
			(*size)++;
			generateLevel(t->right, tokens, size, level);
		break;

		case POWER:
			generateLevel(t->left, tokens, size, level);
			tokens[*size] = t;
			(*size)++;
			if(t->right != NULL)
				generateLevel(t->right, tokens, size, level+1);
		break;
		
		case SIN:
		case ASIN:
		case COS:
		case ACOS:
		case ATAN:
		case LN:
		case SQRT:
		case SEC:
			if(t->right != NULL) {
				generateLevel(t->right, tokens, size, level);
			}
			break;
				
		case LOG:
			generateLevel(t->left, tokens, size, level);

			if(t->right != NULL)
				generateLevel(t->right, tokens, size, level-1);
			break;

		default:
			tokens[*size] = t;
			(*size)++;
		break;
	}
}