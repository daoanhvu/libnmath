#include <stdlib.h>
#include "nlablexer.h"
#include "common.h"
#include "nlabparser.h"

extern int gErrorColumn;
extern int gErrorCode;

int functionNotation1(int index);

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

int clearStackWithoutFreeItem(Token **ls, int len){
	int i;
	//Token *p;

	if(ls==NULL)
		return 0;

	for(i=0; i<len; i++){
		//p = ls[i];
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
			ast->variable = 0;
			ast->value = 0;
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
			ast->variable = 0;
			ast->value = 0;
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

/**
	access right: public
*/	
Function* parseFunctionExpression1(TokenList *tokens){
	int k, l;
	
	gTokens = tokens;
	currentIdx = 0;
	gParenTop = -1;
	gErrorCode = ERROR_NOT_A_FUNCTION;
	gErrorColumn = tokens->list[currentIdx]->column;
	
	if( (k=functionNotation(currentIdx)) > currentIdx ){
		if(tokens->list[k]->type == EQ){
			replaceNAMEByVARIABLE(returnFunction, tokens);
			k++;
			if( (l = expression(k))>k){
				returnFunction->prefix = (NMAST**)malloc(sizeof(NMAST*));
				returnFunction->prefixAllocLen = 1;
				returnFunction->prefixLen = 1;
				returnFunction->prefix[0] = returnedAst;
				returnedAst = NULL;
				
				if( l<tokens->size){
					if( (tokens->list[l]->type == DOMAIN_NOTAION) && (l+1 < tokens->size) && ((k = domain(l+1))>(l+1)) ){
						returnFunction->domain = returnedAst;
						returnedAst = NULL;
						//Clear error status
						gErrorCode = NO_ERROR;
						gErrorColumn = -1;
					}else{
						//ERROR: NOT match domain rule but it has NOT reached the end of token list yet
						gErrorCode = ERROR_BAD_TOKEN;
						gErrorColumn = l;
					}
				}else{
					//Here is the end of token list, it's OK
					//Clear error status
					gErrorCode = NO_ERROR;
					gErrorColumn = -1;
				}
			}
		}
	}
	
	if(gErrorCode != NO_ERROR){
		if(returnFunction != NULL){
			for(k=0;k<returnFunction->prefixLen;k++){
				clearTree(&(returnFunction->prefix[k]));
			}
			
			returnFunction->prefixLen = 0;
			returnFunction->prefixAllocLen = 0;
			free(returnFunction->prefix);
			free(returnFunction);
			returnFunction = NULL;
		}
		
		if(returnedAst != NULL){
			clearTree(&returnedAst);
			returnedAst = NULL;
		}
	}
	
	if(gParenTop > -1){
		gErrorCode = ERROR_PARENTHESE_MISSING;
		gErrorColumn = gTokens->list[gParenStack[gParenTop]]->column;
		if(returnFunction != NULL){
			for(k=0;k<returnFunction->prefixLen;k++){
				clearTree(&(returnFunction->prefix[k]));
			}
			
			returnFunction->prefixLen = 0;
			returnFunction->prefixAllocLen = 0;
			free(returnFunction->prefix);
			free(returnFunction);
			returnFunction = NULL;
		}
	}else if(gParenTop<-1){
		gErrorCode = ERROR_TOO_MANY_PARENTHESE;
		gErrorColumn = gTokens->list[gParenStack[gParenTop]]->column;
		if(returnFunction != NULL){
			for(k=0;k<returnFunction->prefixLen;k++){
				clearTree(&(returnFunction->prefix[k]));
			}
			
			returnFunction->prefixLen = 0;
			returnFunction->prefixAllocLen = 0;
			free(returnFunction->prefix);
			free(returnFunction);
			returnFunction = NULL;
		}
	}
	
	gTokens = NULL;
	return returnFunction;
}

/**
	functionNotation: NAME LPAREN NAME (COMA NAME)* PRARENT;
	@return if
*/
int functionNotation1(int index){
	int i, varsize = 0;
	int oldIndex = index;
	char vars[4];

	if(index >= gTokens->size)
		return index;
	
	if(gTokens->list[index]->type == NAME ){
		gErrorCode = ERROR_PARENTHESE_MISSING;
		gErrorColumn = gTokens->list[index]->column;
		if(gTokens->list[index+1]->type == LPAREN){
			gErrorCode = ERROR_MISSING_VARIABLE;
			gErrorColumn = gTokens->list[index+1]->column;
			
			if(gParenTop<0 || gParenStack[gParenTop] != (index + 1)){
				gParenTop++;
				gParenStack[gParenTop] = index+1;
			}
			
			if(gTokens->list[index+2]->type == NAME){
				vars[varsize++] = (gTokens->list[index+2])->text[0];
				index += 3;
				while( (index+1<gTokens->size) && (gTokens->list[index]->type == COMMA)
							&& (gTokens->list[index+1]->type == NAME ) ){
					vars[varsize++] = (gTokens->list[index+1])->text[0];
					index += 2;
				}
				gErrorCode = ERROR_PARENTHESE_MISSING;
				gErrorColumn = gTokens->list[index]->column;
				if( (index<gTokens->size) && (gTokens->list[index]->type == RPAREN)){
				
					gParenTop--;
				
					returnFunction = (Function*)malloc(sizeof(Function));
					returnFunction->prefix = NULL;
					returnFunction->prefixAllocLen = 0;
					returnFunction->prefixLen = 0;
					returnFunction->domain = NULL;
					returnFunction->str = NULL;
					returnFunction->len = 0;
					returnFunction->variableNode = NULL;
					returnFunction->numVarNode = 0;

					returnFunction->valLen = varsize;
					//Should use memcpy here
					for(i=0;i<varsize;i++)
						returnFunction->variable[i] = vars[i];
						
					gErrorCode = NO_ERROR;
					gErrorColumn = -1;
					return (index + 1);
				}
			}
		}
	}
	gErrorColumn = gTokens->list[index-2]->column;
	return oldIndex;
}//done

/******************************************************************************************/
/**
	Parse the input string in object f to NMAST tree
*/
void parseFunct(TokenList *tokens, int *start, Function *f){
	int i, error, top=-1, allocLen=0, isEndExp = FALSE;
	double val;
	Token *tk = NULL;
	Token **stack = NULL;
	Token *stItm = NULL;
	NMAST *ast = NULL;
	//NMAST* varNodes[50];

	if(tokens == NULL){
		gErrorColumn = 0;
		gErrorCode = ERROR_BAD_TOKEN;
		return ;
	}

	/* Clear prefix tree if any*/
	if(f->prefixLen > 0){
		for(i=0; i<f->prefixLen; i++)
			clearTree(&(f->prefix[i]));
		f->prefixLen = 0;
	}

	f->numVarNode = 0;
	i = (*start);
	while(i < tokens->size && !isEndExp){
		tk = tokens->list[i];
		switch(tk->type){
			case NUMBER:
				val = parseDouble(tk->text, 0, tk->textLength, &error);
				if(val == 0 && error < 0){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARSING_NUMBER;
					return;
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
						ast->priority = stItm->priority;
						ast->left = f->prefix[f->prefixLen-2];
						ast->right = f->prefix[f->prefixLen-1];
						
						if((ast->left)!=NULL)
							(ast->left)->parent = ast;
						if((ast->right)!=NULL)
							(ast->right)->parent = ast;
						
						f->prefix[f->prefixLen-2] = ast;
						f->prefix[f->prefixLen-1] = NULL;
						f->prefixLen--;
						
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

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(stItm == NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return ;
				}

				/*  */
				while(stItm!=NULL && (stItm->type != LPAREN) && isAFunctionType(stItm->type)  != TRUE){
					addFunction2Tree(f, stItm);
					//free(stItm);
					stItm = popFromStack(stack, &top);
				}

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(stItm==NULL){
					free(stack);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return;
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
				//varNodes[f->numVarNode] = ast;
				//(f->numVarNode)++;

				i++;
				break;
				
			case SEMI:
				/** 	End of this expression, kindly stop the while loop, consume this expression and start processing
						the next expression.
				*/
				i++;
				isEndExp = TRUE;
				break;

			default:
				clearStackWithoutFreeItem(stack, top+1);
				free(stack);
				gErrorColumn = tk->column;
				gErrorCode = ERROR_BAD_TOKEN;
				return;
		}//end switch
	}//end while

	while(top >= 0){
		stItm = popFromStack(stack, &top);
		
		if(stItm->type == LPAREN || isAFunctionType(stItm->type)==TRUE){
			free(stItm);
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			gErrorColumn = tk->column;
			gErrorCode = ERROR_PARENTHESE_MISSING;
			return; 
		}
		addFunction2Tree(f, stItm);
	}
	free(stack);
	*start = i;

	//if(f->numVarNode > 0) {
	//	f->variableNode = (NMAST**)malloc(sizeof(NMAST*) * f->numVarNode);
	//	for(i=0; i<f->numVarNode; i++){
	//		f->variableNode[i] = varNodes[i];
	//	}
	//}
}

/*
	Parse the input string in object f to NMAST tree
*/
void parseFunction(Function *f){
	TokenList lst;
	int i, start = 0;

	lst.loggedSize = 10;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;

	/* build the tokens list from the input string */
	parseTokens(f->str, f->len, &lst);
	/* after lexer work, getLexerError() will return -1 if every ok, otherwise it return -1 */
	if( gErrorCode == NO_ERROR ){
		parseFunct(&lst, &start, f);
		for(i = 0; i<lst.size; i++)
			free(lst.list[i]);
	}
	free(lst.list);
}