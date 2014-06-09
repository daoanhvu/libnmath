#include <stdlib.h>
#include "nlablexer.h"
#include "common.h"
#include "nlabparser.h"

#define INCLEN 10

#ifndef _WINDOWS
	#define TRUE -1
	#define FALSE 0
#endif

/******************************************************************************************/
void addItem2Prefix(Function *f, NMAST *item){
	if(f==NULL)
		return;

	if( f->prefixLen >= f->prefixAllocLen ){
		f->prefixAllocLen += INCLEN;
		f->prefix = (NMAST**)realloc(f->prefix, sizeof(NMAST*) * (f->prefixAllocLen) );
	}

	f->prefix[f->prefixLen] = item;
	f->prefixLen++;
}

void clearTree(NMAST **prf){
	
	if((*prf) == NULL)
		return;
	
	if((*prf)->left != NULL)
		clearTree(&(*prf)->left);
	if((*prf)->right != NULL)
		clearTree(&(*prf)->right);
		
	free(*prf);
	(*prf) = NULL;
}

int clearStackWithoutFreeItem(Token **ls, int len){
	int i;
	Token *p;

	if(ls==NULL)
		return 0;

	for(i=0; i<len; i++){
		p = ls[i];
		ls[i] = NULL;
	}
	return i;
}

void pushItem2Stack(Token ***st, int *top, int *allocLen, Token *item){
	if( (*top) >= ( (*allocLen)-1)){
		(*allocLen) += INCLEN;
		(*st) = (Token**)realloc(*st, sizeof(Token*) * (*allocLen) );
	}
	(*top)++;
	(*st)[(*top)] = item;
}

Token* popFromStack(Token **st, int *top){
	Token *p;
	if(st == NULL || ( (*top) < 0))
		return NULL;
	p = st[(*top)];
	st[(*top)] = NULL;
	(*top)--;
	return p;

}

void addFunction2Tree(Function *f, Token * stItm){
	NMAST *ast = NULL;
	
	switch(stItm->type){
		case PLUS:
		case MINUS:
		case MULTIPLY:
		case DIVIDE:
		case POWER:
			ast = (NMAST*)malloc(sizeof(NMAST));
			ast->valueType = TYPE_FLOATING_POINT;
			ast->sign = 1;
			ast->type = stItm->type;
			//ast->priority = stItm->priority;
			ast->parent = NULL;
			ast->left = f->prefix[f->prefixLen-2];
			ast->right = f->prefix[f->prefixLen-1];
			if((f->prefix[f->prefixLen-2])!=NULL)
				(f->prefix[f->prefixLen-2])->parent = ast;
			if((f->prefix[f->prefixLen-1])!=NULL)
				(f->prefix[f->prefixLen-1])->parent = ast;
				
			f->prefix[f->prefixLen-2] = ast;
			f->prefix[f->prefixLen-1] = NULL;
			f->prefixLen--;
		break;

		case SIN:
		case COS:
		case TAN:
		case COTAN:
		case ASIN:
		case ACOS:
		case ATAN:
		case SQRT:
		case LN:
			ast = (NMAST*)malloc(sizeof(NMAST));
			ast->valueType = TYPE_FLOATING_POINT;
			ast->sign = 1;
			ast->type = stItm->type;
			//ast->priority = stItm->priority;
			ast->parent = NULL;
			ast->left = NULL;
			ast->right = f->prefix[f->prefixLen-1];
			if((f->prefix[f->prefixLen-1])!=NULL)
				(f->prefix[f->prefixLen-1])->parent = ast;
				
			f->prefix[f->prefixLen-1] = ast;
		break;
										
		case LOG:
			ast = (NMAST*)malloc(sizeof(NMAST));
			ast->valueType = TYPE_FLOATING_POINT;
			ast->sign = 1;
			ast->type = stItm->type;
			//ast->priority = stItm->priority;
			ast->parent = NULL;
			ast->left = f->prefix[f->prefixLen-2];
			ast->right = f->prefix[f->prefixLen-1];
			if((f->prefix[f->prefixLen-2])!=NULL)
				(f->prefix[f->prefixLen-2])->parent = ast;
			if((f->prefix[f->prefixLen-1])!=NULL)
				(f->prefix[f->prefixLen-1])->parent = ast;
						
			f->prefix[f->prefixLen-2] = ast;
			f->prefix[f->prefixLen-1] = NULL;
			f->prefixLen--;
		break;
	}
}

/******************************************************************************************/

int parseFunct(TokenList *tokens, Function *f, int *idxE){
	int i=0, j =0, error, top=-1, allocLen=0;
	double val;
	Token *tk = NULL;
	Token **stack = NULL;
	Token *stItm = NULL;
	NMAST *ast = NULL;

	NMAST* varNodes[50];

	if(tokens == NULL)
		return -1;

	/* Clear prefix tree if any*/
	if(f->prefixLen > 0){
		for(i=0; i<f->prefixLen; i++)
			clearTree(&(f->prefix[i]));
		f->prefixLen = 0;
	}

	f->numVarNode = 0;

	while(i < tokens->size){
		*idxE = i;
		tk = tokens->list[i];
		switch(tk->type){
			case NUMBER:
				val = parseDouble(tk->text, 0, tk->testLength, &error);

				if(val == 0 && error < 0){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					*idxE = j;
					return error;
				}
				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->valueType = TYPE_FLOATING_POINT;
				ast->sign = 1;
				ast->left = ast->right = ast->parent = NULL;
				ast->value = val;
				ast->type = tk->type;
				addItem2Prefix(f, ast);
				i++;
				break;

			case E_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->left = ast->right = ast->parent = NULL;
				ast->valueType = 0;
				ast->value = E;
				ast->type = E_TYPE;
				addItem2Prefix(f, ast);
				i++;
				break;

			case PI_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->left = ast->right = ast->parent = NULL;
				ast->value = PI;
				ast->type = PI_TYPE;
				ast->valueType = 0;
				addItem2Prefix(f, ast);
				i++;
				break;
				

			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
			case POWER:
				if(top >= 0){
					stItm = stack[top];
					while((isAnOperatorType(stItm->type)==TRUE) && (stItm->priority) >= tk->priority){
						stItm = popFromStack(stack, &top);

						ast = (NMAST*)malloc(sizeof(NMAST));
						ast->left = ast->right = NULL;
						ast->type = stItm->type;
						//ast->priority = stItm->priority;
						ast->left = f->prefix[f->prefixLen-2];
						ast->right = f->prefix[f->prefixLen-1];
						
						if((ast->left)!=NULL)
							(ast->left)->parent = ast;
						if((ast->right)!=NULL)
							(ast->right)->parent = ast;
						
						f->prefix[f->prefixLen-2] = ast;
						f->prefix[f->prefixLen-1] = NULL;
						f->prefixLen--;
						//free(stItm);
						if(top < 0)
							break;

						stItm = stack[top];
					}
				}
				//push operation o1 (tk) into stack
				pushItem2Stack(&stack, &top, &allocLen, tk);
				i++;
				break;

			case LPAREN:/*If it an open parentheses then put it to stack*/
				pushItem2Stack(&stack, &top, &allocLen, tk);
				i++;
				break;

			case RPAREN:
				stItm = popFromStack(stack, &top);

				if(stItm == NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					return ERROR_SYNTAX;
				}

				/*  */
				while(stItm!=NULL && (stItm->type != RPAREN) && isAFunctionType(stItm->type)  != TRUE){
					addFunction2Tree(f, stItm);
					//free(stItm);
					stItm = popFromStack(stack, &top);
				}

				/*check if Open parenthese missing*/
				if(stItm==NULL){
					free(stack);
					return ERROR_PARENTHESE_MISSING;
				}

				if(isAFunctionType(stItm->type)  == TRUE){
					addFunction2Tree(f, stItm);
				}
				//free(stItm);
				i++;
				break;

			//functions
			case SIN:
			case COS:
			case TAN:
			case COTAN:
			case ASIN:
			case ACOS:
			case ATAN:
			case SQRT:
			case LN:
			case LOG:
				pushItem2Stack(&stack, &top, &allocLen, tk);
				/**
					After a function name must be a LPAREN, and we just ignore that LPAREN token
				*/
				i += 2;
				break;

			//case NAME:
			case VARIABLE:
				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->parent = ast->left = ast->right = NULL;
				ast->type = tk->type;
				ast->variable = tk->text[0];
				addItem2Prefix(f, ast);

				//I save variable node to speed up the process of calculating value of the function later
				varNodes[f->numVarNode] = ast;
				(f->numVarNode)++;

				i++;
				break;

			default:
				clearStackWithoutFreeItem(stack, top+1);
				free(stack);
				return ERROR_BAD_TOKEN;
		}//end switch
	}//end while

	while(top >= 0){
		stItm = popFromStack(stack, &top);
		
		if(stItm->type == LPAREN || isAFunctionType(stItm->type)==TRUE){
			free(stItm);
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			return ERROR_PARENTHESE_MISSING; 
		}
		
		addFunction2Tree(f, stItm);
		//free(stItm);
	}
	free(stack);

	if(f->numVarNode > 0){
		f->variableNode = (NMAST**)malloc(sizeof(NMAST*) * f->numVarNode);
		for(i=0; i<f->numVarNode; i++){
			f->variableNode[i] = varNodes[i];
		}
	}

	return 0;
}

int parseFunction(Function *f, int *idxE){
	TokenList lst;
	int i, ret;

	lst.loggedSize = 10;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;
	parseTokens(f->str, f->len, &lst);
	(*idxE) = getError();
	if((*idxE) < 0 ){
		ret = parseFunct(&lst, f, idxE);
		for(i = 0; i<lst.size; i++)
			free(lst.list[i]);
		free(lst.list);
		return ret;
	}

	return (*idxE);
}