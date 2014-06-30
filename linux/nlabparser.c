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
int domain(int index);
int interval(int idx);
int simpleInterval(int idx);
NMAST* parseOR_AND( int index);
int simpleDomain(int index);
int intervalElementOf(int idx);
int intervalWithBoundaries(int idx);

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
				
				if( (tokens->list[k]->type == DOMAIN_NOTATION) 
							&& (k+1 < tokens->size) && ((l = domain(k+1))>(k+1)) ){
					pushASTStack(d, returnedAst);
					returnedAst = NULL;
					//Clear error status
					gErrorCode = NO_ERROR;
					gErrorColumn = -1;
					k = l;
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
			free(stItm);
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

/**
 * interval: intervalWithBoundaries | intervalElementOf | simpleInterval;
 * @return the next index token after the interval
 */
int interval(int idx){
	int nextIdx;
	if( (nextIdx = intervalWithBoundaries(idx)) > idx ){
		return nextIdx;
	}else if( (nextIdx = intervalElementOf(idx)) > idx){
		return nextIdx;
	}else if( (nextIdx = simpleInterval(idx)) > idx){
		return nextIdx;
	}
	return idx;
}//done
	
/**
 * simpleInterval : LPAREN? (NAME|VARIABLE) (LT | LTE | GT | GTE) (NUMBER|PI_TYPE|E_TYPE) RPARENT?
 * @param idx
 * @return
 */
int simpleInterval(int idx){
	Token *tk;
	int oldIdx = idx;
	
	if(idx >= gTokens->size) return idx;
	
	tk = gTokens->list[idx];
	if(tk->type == LPAREN && ( gParenTop<0 || gParenStack[gParenTop]!=idx )){
		gParenTop++;
		gParenStack[gParenTop]=idx;
		idx++;
		tk = gTokens->list[idx];
	}
		
	if(tk->type == NAME || tk->type == VARIABLE){
		if(gTokens->list[idx+1]->type==LT ||
				gTokens->list[idx+1]->type==LTE || gTokens->list[idx+1]->type==GT ||
				gTokens->list[idx+1]->type==GTE) {
			if(gTokens->list[idx+2]->type == NUMBER || gTokens->list[idx+2]->type == PI_TYPE 
										|| gTokens->list[idx+2]->type == E_TYPE) {
				if( (idx + 3)<gTokens->size && gTokens->list[idx + 3]->type == RPAREN){
					if(gParenTop < 0) {
						return oldIdx;
					}
					gParenTop--;
					idx++;
				}
					
				returnedAst = (NMAST*)malloc(sizeof(NMAST));
				returnedAst->parent = NULL;
				returnedAst->type = gTokens->list[idx+1]->type;
				returnedAst->variable = 0;
				returnedAst->value = 0;
				returnedAst->priority = gTokens->list[idx+1]->priority;
				
				returnedAst->left = (NMAST*)malloc(sizeof(NMAST));
				returnedAst->left->parent = returnedAst;
				returnedAst->left->type = tk->type;
				returnedAst->left->variable = tk->text[0];
				returnedAst->left->value = 0;
				returnedAst->left->left = returnedAst->left->right = NULL;
				
				returnedAst->right = (NMAST*)malloc(sizeof(NMAST));
				returnedAst->right->parent = returnedAst;
				returnedAst->right->type = gTokens->list[idx+2]->type;
				returnedAst->right->variable = 0;
				returnedAst->right->left = returnedAst->right->right = NULL;
				switch(gTokens->list[idx+2]->type){
					case NUMBER:
						returnedAst->right->value = parseDouble(gTokens->list[idx+2]->text, 0, gTokens->list[idx+2]->textLength, &gErrorColumn);
					break;
					
					case PI_TYPE:
						returnedAst->right->value = PI;
					break;
					
					case E_TYPE:
						returnedAst->right->value = E;
					break;
				}
				
				return (idx+3);
			}
		}
	}
		
	return oldIdx;
}//done
	
/**
 * intervalElementOf: NAME ELEMENT_OF (LPAREN | LPRACKET) NUMBER COMMA NUMBER (RPAREN | RPRACKET);
 * @param idx
 * @return
 */
int intervalElementOf(int idx){
	double val1, val2;
	Token *tk, *tokenK1, *tokenK2, *tokenK3, *tokenK4, *tokenK5, *tokenK6;
	
	if(idx >= gTokens->size) return idx;
	
	tk = gTokens->list[idx];
	returnedAst = NULL;
	if(tk->type == NAME){
		tokenK1 = gTokens->list[idx + 1];
		if(tokenK1->type==ELEMENT_OF) {
			tokenK2 = gTokens->list[idx + 2];
			if(tokenK2->type == LPAREN || tokenK2->type == LPRACKET ){
				tokenK3 = gTokens->list[idx + 3];
				if(tokenK3->type == NUMBER || tokenK3->type == PI_TYPE || tokenK3->type == E_TYPE){
					switch(tokenK3->type){
						case NUMBER:
							val1 = parseDouble(tokenK3->text, 0, tokenK3->textLength, &gErrorColumn);
						break;

						case PI_TYPE:
							val1 = PI;
							break;

						case E_TYPE:
							val1 = E;
							break;
					}
					tokenK4 = gTokens->list[idx + 4];
					if(tokenK4->type == COMMA){
						tokenK5 = gTokens->list[idx + 5];
						if(tokenK5->type == NUMBER || tokenK5->type == PI_TYPE || tokenK5->type == E_TYPE){
							switch(tokenK5->type){
								case NUMBER:
									val2 = parseDouble(tokenK5->text, 0, tokenK5->textLength, &gErrorColumn);
								break;

								case PI_TYPE:
									val2 = PI;
									break;

								case E_TYPE:
									val2 = E;
									break;
							}
							tokenK6 = gTokens->list[idx + 6];
							if(tokenK6->type == RPAREN || tokenK6->type == RPRACKET ){
								returnedAst = (NMAST*)malloc(sizeof(NMAST));
								returnedAst->type = GT_LT;
								returnedAst->variable = tk->text[0];
								returnedAst->parent = NULL;

								returnedAst->left = (NMAST*)malloc(sizeof(NMAST));
								returnedAst->left->type = tokenK3->type;
								returnedAst->left->value = val1;
								returnedAst->left->parent = returnedAst;

								returnedAst->right = (NMAST*)malloc(sizeof(NMAST));
								returnedAst->right->type = tokenK5->type;;
								returnedAst->right->value = val2;
								returnedAst->right->parent = returnedAst;

								return (idx + 7);
							}
						}
					}
				}
			}
		}
	}
	return idx;
}
	
/**
 * intervalWithLowerAndUpper: LPAREN? NUMBER (LT | LTE | GT | GTE) (NAME|VARIABLE) (LT | LTE | GT | GTE) NUMBER RPARENT? ;
 *	<br>
 * Note that the tree built in this rule as same as the one in rule intervalElementOf
 * @return
 */
int intervalWithBoundaries(int idx){
	double val1, val2;
	Token *tk;
	Token *tokenK1, *tokenK2, *tokenK3, *tokenK4;
	int oldIdx = idx, type;
	
	if(idx >= gTokens->size) return idx;

	tk = gTokens->list[idx];
	if(tk->type == LPAREN && ( gParenTop<0 || gParenStack[gParenTop] != idx ) ){
		gParenTop++;
		gParenStack[gParenTop] = idx;
		idx++;
		tk = gTokens->list[idx];
	}
		
	if(tk->type == NUMBER || tk->type == PI_TYPE || tk->type == E_TYPE){
	
		switch(tk->type){
			case NUMBER:
				val1 = parseDouble(tk->text, 0, tk->textLength, &gErrorColumn);
			break;
			
			case PI_TYPE:
				val1 = PI;
			break;
			
			case E_TYPE:
				val1 = E;
			break;
		}
		
		tokenK1 = gTokens->list[idx + 1];
		if(isComparationOperator(tokenK1->type)){
			tokenK2 = gTokens->list[idx + 2];
			if(tokenK2->type == NAME || tokenK2->type == VARIABLE){
				tokenK3 = gTokens->list[idx + 3];
				if(isComparationOperator(tokenK3->type)){
					tokenK4 = gTokens->list[idx + 4];
					if(tokenK4->type == NUMBER || tokenK4->type == PI_TYPE || tokenK4->type == E_TYPE){
						
						switch(tokenK4->type){
							case NUMBER:
								val2 = parseDouble(tokenK4->text, 0, tokenK4->textLength, &gErrorColumn);
							break;
							
							case PI_TYPE:
								val2 = PI;
							break;
							
							case E_TYPE:
								val2 = E;
							break;
						}
						
						if(gTokens->list[idx + 5]->type == RPAREN){
							if(gParenTop < 0)
								return oldIdx;

							gParenTop--;
							idx++;
						}
							
						/**
							The reason for doing this is that we always
							read the comparation in the order that 
							variable comes first.
							Example: -1 < x < 3
								x is greater than -1 and x is less than 3
						*/
						if(tokenK1->type == GT && tokenK3->type == LT)
							type = GT_LT;
						else if(tokenK1->type == GTE && tokenK3->type == LT)
							type = GTE_LT;
						else if(tokenK1->type == GT && tokenK3->type == LTE)
							type = GT_LTE;
						else if(tokenK1->type == GTE && tokenK3->type == LTE)
							type = GTE_LTE;
							
						returnedAst = (NMAST*)malloc(sizeof(NMAST));
						returnedAst->type = type;
						returnedAst->variable = tokenK2->text[0];
						returnedAst->parent = NULL;
						
						returnedAst->left = (NMAST*)malloc(sizeof(NMAST));
						returnedAst->left->type = NUMBER;
						returnedAst->left->value = val1;
						returnedAst->left->parent = returnedAst;

						returnedAst->right = (NMAST*)malloc(sizeof(NMAST));
						returnedAst->right->type = NUMBER;
						returnedAst->right->value = val2;
						returnedAst->right->parent = returnedAst;
						
						return idx + 5;
					}
				}
			}
		}
	}
	return oldIdx;
}
	
/**
 * simple_domain: LPAREN? interval ( (OR | AND) interval)* RPAREN? ;
 */
int simpleDomain(int index){
	int k, l;
	int oldIndex = index;
	NMASTList *rs, *sk;
	NMAST *itm, *op1, *op2;
	Token *tk;
	
	if(index >= gTokens->size) return index;
	
	tk = gTokens->list[index];
	if(tk->type == LPAREN && ( gParenTop<0 || gParenStack[gParenTop]!=index ) ){
		gParenTop++;
		gParenStack[gParenTop] = index;
		index++;
	}
		
	returnedAst = NULL;
	if( (k=interval(index)) > index ){
		//init rs
		rs = (NMASTList*)malloc(sizeof(NMASTList));
		rs->list = NULL;
		rs->loggedSize = 0;
		rs->size = 0;
		
		pushASTStack(rs, returnedAst);
		if( k < gTokens->size ) {
			tk = gTokens->list[k];
		}else{
			//If we got a RPAREN here, maybe it belong to parent rule
			//if(index > oldIndex && gParenTop > 0)
			//	gParenTop--;
					
			free(rs);
			return k;
		}
		sk = (NMASTList*)malloc(sizeof(NMASTList));
		sk->list = NULL;
		sk->loggedSize = 0;
		sk->size = 0;

		while( tk != NULL ){
			if((tk->type == OR || tk->type == AND) 
							&& ((l = interval(k+1))>(k+1))) {
				//Logical Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type = gTokens->list[index]->type;
				op1->priority = tk->priority;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				op1->sign = 1;
				op1->parent = op1->left = op1->right = NULL;
				
				while(sk->size>0 && (sk->list[sk->size-1]->type==OR||sk->list[sk->size-1]->type==AND)
						&&(sk->list[sk->size-1]->priority >= op1->priority)){
					op2 = popASTStack(sk);
					pushASTStack(rs, op2);
				}
				pushASTStack(sk, op1);
				
				pushASTStack(rs, returnedAst);
				k = l;
				tk = (k<gTokens->size)?gTokens->list[k]:NULL;
			}else if(tk->type == RPAREN){
				//TODO: need clear stack, clear rs, release stack, release rs
				if(gParenTop < 0){ //ERROR: got RPAREN but there is not any LPAREN
					free(sk);
					free(rs);
					return oldIndex;
				}
				/**
				 * it's is possible that this RPAREN comes from parent rule
				 */
				gParenTop--;
				k++;
				tk = NULL;
			}else{
				tk = NULL;
			}
		}
			
		while(sk->size > 0){
			pushASTStack(rs, popASTStack(sk));
		}
		//release stack
		free(sk);
			
		if(rs->size > 0){
			//Build tree from infix
			while(rs->size > 1){
				l = 2;
				itm = rs->list[l];
				while( (l+1)<rs->size && itm->type != OR && itm->type != AND){
					l++;
					itm = rs->list[l];
				}
				itm->left = rs->list[l-2];
				itm->right = rs->list[l-1];
				removeFromNMASTList(rs, l-2, 2);
			}
			returnedAst = rs->list[0];
		}
		free(rs);	
		return k;
	}
	return oldIndex;
} //done
	
NMAST* parseOR_AND( int index) {
	NMAST* ast;
	if(gTokens->list[index]->type == OR || gTokens->list[index]->type == AND){
		ast = (NMAST*)malloc(sizeof(NMAST)); //AST(currentToken.getType(), .getText(), currentToken.getColumn());
		ast->type = gTokens->list[index]->type;
		ast->value = 0.0;
		ast->priority = gTokens->list[index]->priority;
		ast->valueType = TYPE_FLOATING_POINT;
		ast->sign = 1;
		ast->parent = ast->left = ast->right = NULL;
		return ast;
	}
	return NULL;
}

/**
 * domain: LPARENT? simple_domain ( (OR | AND) simple_domain)*  RPARENT?;
 */
int domain(int index){
	int k, l;
	int oldIndex = index;
	NMASTList *rs, *sk;
	NMAST *itm, *op1, *op2;
	Token *tk;
	
	if(index >= gTokens->size) return index;
	
	tk = gTokens->list[index];
	if(tk->type == LPAREN && ( gParenTop<0 || gParenStack[gParenTop]!=index ) ){
		gParenTop++;
		gParenStack[gParenTop] = index;
		index++;
	}
		
	returnedAst = NULL;
	if( (k=simpleDomain(index)) > index ){
		//init rs
		rs = (NMASTList*)malloc(sizeof(NMASTList));
		rs->list = NULL;
		rs->loggedSize = 0;
		rs->size = 0;
		
		pushASTStack(rs, returnedAst);
		if( k < gTokens->size ) {
			tk = gTokens->list[k];
		}else{
			//If we got a RPAREN here, maybe it belong to parent rule
			//if(index > oldIndex)
			//	g_ParenInStack--;
					
			free(rs);
			return k;
		}
		sk = (NMASTList*)malloc(sizeof(NMASTList));
		sk->list = NULL;
		sk->loggedSize = 0;
		sk->size = 0;
			
		while( tk != NULL ){
			if((tk->type==OR||tk->type==AND) && ((l = simpleDomain(k+1))>(k+1))) {
				//Logical Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type = gTokens->list[index]->type;
				op1->priority = tk->priority;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				op1->sign = 1;
				op1->parent = op1->left = op1->right = NULL;
				
				while(sk->size>0 && (sk->list[sk->size-1]->type==OR||sk->list[sk->size-1]->type==AND)
						&&(sk->list[sk->size-1]->priority >= tk->priority)){
					op2 = popASTStack(sk);
					pushASTStack(rs, op2);
				}
				pushASTStack(sk, op1);
				
				pushASTStack(rs, returnedAst);
				k = l;
				tk = (k<gTokens->size)?gTokens->list[k]:NULL;
			}else if(tk->type == RPAREN){
				//TODO: need clear stack, clear rs, release stack, release rs
				if(gParenTop < 0){ //ERROR:
					free(sk);
					free(rs);
					return oldIndex;
				}
				/**
				 * it's is possible that this RPAREN comes from parent rule
				 */
				gParenTop--;
				k++;
				tk = NULL;
			}else{
				tk = NULL;
			}
		}
			
		while(sk->size > 0){
			pushASTStack(rs, popASTStack(sk));
		}
		//release stack
		free(sk);
			
		if(rs->size > 0){
			//Build tree from infix
			while(rs->size > 1){
				l = 2;
				itm = rs->list[l];
				while(itm->type != OR && itm->type != AND){
					l++;
					itm = rs->list[l];
				}
				itm->left = rs->list[l-2];
				itm->right = rs->list[l-1];
				removeFromNMASTList(rs, l-2, 2);
			}
			returnedAst = rs->list[0];
		}
		free(rs);	
		return k;
	}
	return oldIndex;
}