#include <stdlib.h>
#include "nlablexer.h"
#include "common.h"
#include "nlabparser.h"

extern int gErrorColumn;
extern int gErrorCode;
extern TokenList *gTokens;

//internal variables
short gParenStack[100];
short gParenTop;
Function *returnFunction = NULL;
NMAST *returnedAst = NULL;

int functionNotation1(int index);
void parseFunct(TokenList *tokens, int *start, Function *f);
void domain(int *start, Function *f);

/******************************************************************************************/
/**
	Special routine used for PRN, just use internally please
*/
void removeFromNMASTList(NMASTList *sk, int fromIdx, int len){
	int i;
	if(sk == NULL || fromIdx<0 || fromIdx >= sk->size)
		return;

	for(i=fromIdx+len; i<sk->size; i++){
		sk->list[i-len] = sk->list[i];
		sk->list[i] = NULL;
	}
	(sk->size) -= len;
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

void addFunction2Tree(NMASTList *t, Token * stItm){
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
			ast->left = t->list[t->size-2];
			ast->right = t->list[t->size-1];
			if((t->list[t->size-2])!=NULL)
				(t->list[t->size-2])->parent = ast;
			if((t->list[t->size-1])!=NULL)
				(t->list[t->size-1])->parent = ast;
				
			t->list[t->size-2] = ast;
			t->list[t->size-1] = NULL;
			t->size--;
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
			ast->right = t->list[t->size-1];
			if((t->list[t->size-1])!=NULL)
				(t->list[t->size-1])->parent = ast;
				
			t->list[t->size-1] = ast;
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
			ast->left = t->list[t->size-2];
			ast->right = t->list[t->size-1];
			if((t->list[t->size-2])!=NULL)
				(t->list[t->size-2])->parent = ast;
			if((t->list[t->size-1])!=NULL)
				(t->list[t->size-1])->parent = ast;
						
			t->list[t->size-2] = ast;
			t->list[t->size-1] = NULL;
			t->size--;
		break;
	}
}

/**
	access right: public
*/	
Function* parseFunctionExpression1(TokenList *tokens){
	int k, l, i, idx = 0;
	NMASTList *d;
	gTokens = tokens;
	gErrorCode = ERROR_NOT_A_FUNCTION;
	gErrorColumn = tokens->list[idx]->column;
	d = (NMASTList*)malloc(sizeof(NMASTList));
	d->size = 0;
	d->loggedSize = 0;
	d->list = NULL;
	
	if( (k=functionNotation1(idx)) > idx ){
		if(tokens->list[k]->type == EQ){
		
			for(i=0; i<tokens->size; i++){
				if(tokens->list[i]->type == NAME){
					for(l=0; l<returnFunction->valLen; l++){
						if(tokens->list[i]->text[0]==returnFunction->variable[l] 
										&& tokens->list[i]->textLength==1)
							tokens->list[i]->type = VARIABLE;
					}
				}
			}
			
			k++;
			do{
				parseFunct(tokens, &k, returnFunction);
				
				if(k >= tokens->size) break;
				
				if(tokens->list[k]->type == DOMAIN_NOTATION){
					if(k+1 < tokens->size){
						l = k + 1;
						domain(&l, returnFunction);
						k = l;
					} else{
						//Here, we got a DOMAIN_NOTATION but it's at the end of the token list
					}
				}
			}while( gErrorCode==NO_ERROR && k < tokens->size );
		}
	}
	
	if(gErrorCode != NO_ERROR){
		if(returnFunction != NULL){
			for(k=0; k<returnFunction->prefix->size; k++){
				clearTree(&(returnFunction->prefix->list[k]));
			}
			returnFunction->prefix->size = 0;
			free(returnFunction->prefix);
			
			for(k=0; k<returnFunction->domain->size; k++){
				clearTree(&(returnFunction->domain->list[k]));
			}
			returnFunction->domain->size = 0;
			free(returnFunction->domain);
			
			free(returnFunction);
			returnFunction = NULL;
		}
		
		if(returnedAst != NULL){
			clearTree(&returnedAst);
			returnedAst = NULL;
		}
	}
	gTokens = NULL;
	returnFunction->domain = d;
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
					returnFunction = (Function*)malloc(sizeof(Function));
					returnFunction->prefix = NULL;
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
	NMASTList *prefix;
	Token *stItm = NULL;
	NMAST *ast = NULL;
	//NMAST* varNodes[50];

	if(tokens == NULL){
		gErrorColumn = 0;
		gErrorCode = ERROR_BAD_TOKEN;
		return ;
	}
	prefix = (NMASTList*)malloc(sizeof(NMASTList));
	prefix->size = 0;
	prefix->loggedSize = 0;
	prefix->list = NULL;
	
	gErrorColumn = -1;
	gErrorCode = NO_ERROR;
	
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
					for(i=0;i<prefix->size;i++)
						clearTree(&(prefix->list[i]));
					free(prefix->list);
					free(prefix);
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
				pushASTStack(prefix, ast); //add this item to prefix
				i++;
				break;

			case E_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->left = ast->right = ast->parent = NULL;
				ast->valueType = 0;
				ast->value = E;
				ast->type = E_TYPE;
				pushASTStack(prefix, ast); //add this item to prefix
				i++;
				break;

			case PI_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->left = ast->right = ast->parent = NULL;
				ast->value = PI;
				ast->type = PI_TYPE;
				ast->valueType = 0;
				pushASTStack(prefix, ast); //add this item to prefix
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
						ast->left = prefix->list[prefix->size-2];
						ast->right = prefix->list[prefix->size-1];
						
						if((ast->left)!=NULL)
							(ast->left)->parent = ast;
						if((ast->right)!=NULL)
							(ast->right)->parent = ast;
						
						prefix->list[prefix->size-2] = ast;
						prefix->list[prefix->size-1] = NULL;
						prefix->size--;
						
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
					for(i=0;i<prefix->size;i++)
						clearTree(&(prefix->list[i]));
					free(prefix->list);
					free(prefix);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return ;
				}

				/*  */
				while(stItm!=NULL && (stItm->type != LPAREN) && isAFunctionType(stItm->type)  != TRUE){
					addFunction2Tree(prefix, stItm);
					//free(stItm);
					stItm = popFromStack(stack, &top);
				}

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(stItm==NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for(i=0;i<prefix->size;i++)
						clearTree(&(prefix->list[i]));
					free(prefix->list);
					free(prefix);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return;
				}

				if(isAFunctionType(stItm->type)  == TRUE){
					addFunction2Tree(prefix, stItm);
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
				pushASTStack(prefix, ast); //add this item to prefix

				//I save variable node to speed up the process of calculating value of the function later
				//varNodes[f->numVarNode] = ast;
				//(f->numVarNode)++;

				i++;
				break;
				
			case SEMI:
			case DOMAIN_NOTATION:
				/** 	End of this expression, kindly stop the while loop, consume 
						this expression and start processing the next expression.
				*/
				isEndExp = TRUE;
				break;

			default:
				clearStackWithoutFreeItem(stack, top+1);
				free(stack);
				for(i=0;i<prefix->size;i++)
					clearTree(&(prefix->list[i]));
				free(prefix->list);
				free(prefix);
				gErrorColumn = tk->column;
				gErrorCode = ERROR_BAD_TOKEN;
				return;
		}//end switch
	}//end while

	while(top >= 0){
		stItm = popFromStack(stack, &top);
		
		if(stItm->type == LPAREN || isAFunctionType(stItm->type)==TRUE){
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			for(i=0;i<prefix->size;i++)
				clearTree(&(prefix->list[i]));
			free(prefix->list);
			free(prefix);
			gErrorColumn = tk->column;
			gErrorCode = ERROR_PARENTHESE_MISSING;
			return; 
		}
		addFunction2Tree(prefix, stItm);
	}
	
	/**
		If we make sure that the function f is initialized 
		ok we do not need to check here. Actually, we SHOULD initialize it completely
	*/
	if(f->prefix == NULL)
		f->prefix = (NMASTList*)malloc(sizeof(NMASTList));
		
	pushASTStack(f->prefix, prefix->list[0]);
	
	free(stack);
	free(prefix);
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
Function* parseFunction(char *str, int len){
	TokenList lst;
	int i;
	Function *f = NULL;
	
	lst.loggedSize = 10;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;

	/* build the tokens list from the input string */
	parseTokens(str, len, &lst);
	/* after lexer work, getLexerError() will return -1 if every ok, otherwise it return -1 */
	
	if( gErrorCode == NO_ERROR ){
		f = parseFunctionExpression1(&lst);
		for(i = 0; i<lst.size; i++)
			free(lst.list[i]);
	}
	free(lst.list);
	
	return f;
}

void domain(int *start, Function *f){
	int isEndExp = FALSE;
	int i, index, top = -1, allocLen=0;
	Token* tk;
	double val;
	Token **stack = NULL;
	NMASTList *d;
	Token *tokenItm = NULL;
	NMAST *ast, *astTmp;

	if(gTokens == NULL){
		gErrorColumn = 0;
		gErrorCode = ERROR_BAD_TOKEN;
		return ;
	}
	d = (NMASTList*)malloc(sizeof(NMASTList));
	d->size = 0;
	d->loggedSize = 0;
	d->list = NULL;
	
	gErrorColumn = -1;
	gErrorCode = NO_ERROR;
	
	index = *start;
	while(index < gTokens->size && !isEndExp){
		tk = gTokens->list[index];
		switch(tk->type){
			case NUMBER:
			case PI_TYPE:
			case E_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
				val = parseDouble(tk->text, 0, tk->textLength, &gErrorCode);
				if(val == 0 && gErrorCode != NO_ERROR){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
					gErrorColumn = tk->column;
					return;
				}

				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->valueType = TYPE_FLOATING_POINT;
				ast->sign = 1;
				ast->left = ast->right = ast->parent = NULL;
				ast->value = val;
				ast->type = tk->type;
				pushASTStack(d, ast);
				index++;
			break;
			
			case LT:
			case LTE:
			case GT:
			case GTE:
			case AND:
			case OR:
			if(top >= 0){
					tokenItm = stack[top];
					while((isAnOperatorType(tokenItm->type)==TRUE) && (tokenItm->priority) >= tk->priority){
						tokenItm = popFromStack(stack, &top);

						ast = (NMAST*)malloc(sizeof(NMAST));
						ast->left = ast->right = NULL;
						ast->type = tokenItm->type;
						ast->priority = tokenItm->priority;
						ast->left = d->list[d->size-2];
						ast->right = d->list[d->size-1];
						
						if((ast->left)!=NULL)
							(ast->left)->parent = ast;
						if((ast->right)!=NULL)
							(ast->right)->parent = ast;
						
						d->list[d->size-2] = ast;
						d->list[d->size-1] = NULL;
						d->size--;
						
						if(top < 0)
							break;

						tokenItm = stack[top];
					}
				}
				//push operation o1 (tk) into stack
				pushItem2Stack(&stack, &top, &allocLen, tk);
				index++;
			break;

			case RPAREN:
				tokenItm = popFromStack(stack, &top);

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(tokenItm == NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return ;
				}

				/*  */
				while(tokenItm!=NULL && (tokenItm->type != LPAREN) && isAFunctionType(tokenItm->type)  != TRUE){
					addFunction2Tree(d, tokenItm);
					tokenItm = popFromStack(stack, &top);
				}

				if(tokenItm==NULL){
					/** ERROR: got an opening-parenthese but can not find a closing-parenthese */
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return;
				}

				if(isAFunctionType(tokenItm->type)  == TRUE){
					addFunction2Tree(d, tokenItm);
				}
				
				index++;
			break;
				
			case LPAREN:
				pushItem2Stack(&stack, &top, &allocLen, tk);
				index++;
			break;
			
			case VARIABLE:
				if(( (index+1) < gTokens->size) && gTokens->list[index+1]->type == ELEMENT_OF){
					/*
						VARIABLE ELEMENT_OF [NUMBER,NUMBER]
						VARIABLE ELEMENT_OF (NUMBER,NUMBER]
						VARIABLE ELEMENT_OF [NUMBER,NUMBER)
						VARIABLE ELEMENT_OF (NUMBER,NUMBER)
					*/
					if( ( index+6 < gTokens->size) && (gTokens->list[index+2]->type == LPRACKET || gTokens->list[index+2]->type == LPAREN)
								&& (gTokens->list[index+3]->type == NUMBER || gTokens->list[index+3]->type == PI_TYPE || gTokens->list[index+3]->type == E_TYPE)
								&& gTokens->list[index+4]->type == COMMA 
								&& (gTokens->list[index+5]->type == NUMBER || gTokens->list[index+5]->type == PI_TYPE || gTokens->list[index+5]->type == E_TYPE) 
								&& (gTokens->list[index+6]->type == RPRACKET || gTokens->list[index+6]->type == RPAREN )){
								
						ast = (NMAST*)malloc(sizeof(NMAST));
						ast->valueType = TYPE_FLOATING_POINT;
						ast->sign = 1;
						ast->left = ast->right = ast->parent = NULL;
						ast->value = val;
						ast->type = AND;
						
						//Left GTE or GT
						astTmp = (NMAST*)malloc(sizeof(NMAST));
						astTmp->valueType = TYPE_FLOATING_POINT;
						astTmp->sign = 1;
						astTmp->left = astTmp->right = NULL;
						astTmp->parent = ast;
						astTmp->value = val;
						astTmp->type = (gTokens->list[index+2]->type == LPRACKET)?GTE:GT;
						ast->left = astTmp;
						
						//Left->Left VARIABLE
						astTmp->left = (NMAST*)malloc(sizeof(NMAST));
						astTmp->left->valueType = TYPE_FLOATING_POINT;
						astTmp->left->sign = 1;
						astTmp->left->left = astTmp->left->right = NULL;
						astTmp->left->parent = astTmp;
						astTmp->left->value = 0.0;
						astTmp->left->type = VARIABLE;
						
						//Left->Right NUMBER or PI_TYPE or E_TYPE
						astTmp->right = (NMAST*)malloc(sizeof(NMAST));
						astTmp->right->valueType = TYPE_FLOATING_POINT;
						astTmp->right->sign = 1;
						astTmp->right->left = astTmp->right->right = NULL;
						astTmp->right->parent = astTmp;
						switch(gTokens->list[index+3]->type){
							case NUMBER:
								astTmp->right->value = parseDouble(gTokens->list[index+3]->text, 0, gTokens->list[index+3]->textLength, &gErrorCode);
							break;
							
							case PI_TYPE:
								astTmp->right->value = PI;
							break;
							
							case E_TYPE:
								astTmp->right->value = E;
							break;
						}
						astTmp->right->type = gTokens->list[index+3]->type;
						
						
						//Right
						astTmp = (NMAST*)malloc(sizeof(NMAST));
						astTmp->valueType = TYPE_FLOATING_POINT;
						astTmp->sign = 1;
						astTmp->left = astTmp->right = NULL;
						astTmp->parent = ast;
						astTmp->value = val;
						astTmp->type = (gTokens->list[index+6]->type == RPRACKET)?LTE:LT;
						ast->right = astTmp;
						
						//Right->Left VARIABLE
						astTmp->left = (NMAST*)malloc(sizeof(NMAST));
						astTmp->left->valueType = TYPE_FLOATING_POINT;
						astTmp->left->sign = 1;
						astTmp->left->left = astTmp->left->right = NULL;
						astTmp->left->parent = astTmp;
						astTmp->left->value = 0.0;
						astTmp->left->type = VARIABLE;
						
						//Right->Right NUMBER or PI_TYPE or E_TYPE
						astTmp->right = (NMAST*)malloc(sizeof(NMAST));
						astTmp->right->valueType = TYPE_FLOATING_POINT;
						astTmp->right->sign = 1;
						astTmp->right->left = astTmp->right->right = NULL;
						astTmp->right->parent = astTmp;
						switch(gTokens->list[index+5]->type){
							case NUMBER:
								astTmp->right->value = parseDouble(gTokens->list[index+3]->text, 0, gTokens->list[index+3]->textLength, &gErrorCode);
							break;
							
							case PI_TYPE:
								astTmp->right->value = PI;
							break;
							
							case E_TYPE:
								astTmp->right->value = E;
							break;
						}
						astTmp->right->type = gTokens->list[index+5]->type;
						
						pushASTStack(d, ast);
						index += 7;		
					}else{
						/**
							ERROR:After ELEMENT_OF is not a right syntax
						*/
						clearStackWithoutFreeItem(stack, top+1);
						free(stack);
						for(i=0;i<d->size;i++)
							clearTree(&(d->list[i]));
						free(d->list);
						free(d);
						gErrorColumn = tk->column;
						gErrorCode = ERROR_SYNTAX;
						return; 
					}
				}else {
					ast = (NMAST*)malloc(sizeof(NMAST));
					ast->valueType = TYPE_FLOATING_POINT;
					ast->sign = 1;
					ast->left = ast->right = ast->parent = NULL;
					ast->value = val;
					ast->type = tk->type;
					pushASTStack(d, ast);
					index++;
				}
			break;
			
			case SEMI:
				isEndExp = TRUE;
			break;
		}
	}
	
	while(top >= 0){
		tokenItm = popFromStack(stack, &top);
		
		if(tokenItm->type == LPAREN || isAFunctionType(tokenItm->type)==TRUE){
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			for(i=0;i<d->size;i++)
				clearTree(&(d->list[i]));
			free(d->list);
			free(d);
			gErrorColumn = tk->column;
			gErrorCode = ERROR_PARENTHESE_MISSING;
			return; 
		}
		addFunction2Tree(d, tokenItm);
	}
	*start = index;
	/**
		If we make sure that the function f is initialized 
		ok we do not need to check here. Actually, we SHOULD initialize it completely
	*/
	if(f->domain == NULL)
		f->domain = (NMASTList*)malloc(sizeof(NMASTList));
		
	pushASTStack(f->domain, d->list[0]);
	
	free(stack);
	free(d);
}