#include <stdlib.h>
#include <stdio.h>
#include "nlabparser2.h"

#define MAXPOOLSIZE 32

/**
	TODO: check priority attribute of NMAST object
*/

/** Global variables those are used in parsing */
int currentIdx = -1;
Function *returnFunction = NULL;
NMAST *returnedAst = NULL;

short gParenStack[100];
short gParenTop;

extern int gErrorColumn;
extern int gErrorCode;
extern TokenList *gTokens;

void replaceNAMEByVARIABLE(Function *f, TokenList *tokens){
	int i, k;
	for(i=0; i<tokens->size; i++){
		if(tokens->list[i]->type == NAME){
			for(k=0; k<f->valLen; k++){
				if(tokens->list[i]->text[0]==f->variable[k] && tokens->list[i]->textLength==1)
					tokens->list[i]->type = VARIABLE;
			}
		}
	}
}

/**
	access right: public
*/	
Function* parseFunctionExpression(TokenList *tokens){
	Function* fd = NULL;
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
	access right: public
*/	
int parseExpression() {
	return 0;
}
/**
	functionNotation: NAME LPAREN NAME (COMA NAME)* PRARENT;
	@return if
*/
int functionNotation(int index){
	int i, varsize = 0;
	int oldIndex = index;
	Token *next = NULL;
	NMAST *functionDefNode = NULL;
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

NMAST* popASTStack(NMASTList *sk){
	NMAST* ele;
	if(sk == NULL || sk->size == 0)
		return NULL;
	ele = sk->list[sk->size-1];
	(sk->size)--;
	return ele;
}

void pushASTStack(NMASTList *sk, NMAST* ele){
	//NMAST **lst;
	if(sk == NULL)
		return;
		
	if(sk->size == sk->loggedSize){
		sk->loggedSize += INCLEN;
		/**
			IMPORTANT: It may not secure here
		*/
		sk->list = (NMAST**)realloc(sk->list, sizeof(NMAST*) * sk->loggedSize);
		
		//if(lst != NULL)
		//	sk->list = lst;
	}
	sk->list[sk->size] = ele;
	(sk->size)++;
}

NMAST* removeNMASTAt(NMASTList *sk, int k){
	int i;
	NMAST* ele;
	if(sk == NULL || k<0 || k >= sk->size)
		return;
	
	ele = sk->list[k];
	for(i=k+1; i<sk->size; i++){
		sk->list[i-1] = sk->list[i];
	}
	sk->list[sk->size - 1] = NULL;
	(sk->size)--;
	
	return ele;
}

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

/**
 * expression: LPAREN? expressionWithoutParenthese (( + | - | * | / | ^) expressionWithoutParenthese)* LPAREN?
 * @param index
 * @return
 */
int expression(int index){
	int k, l;
	int oldIndex = index;
	NMASTList *rs, *sk;
	NMAST *itm, *op1, *op2;
	
	NMAST	*pool[MAXPOOLSIZE];
	int poolSize = 0;
	
	Token *tk;
	
	if(index >= gTokens->size) return index;
	
	tk = gTokens->list[index];
	if(tk->type == LPAREN && (gParenTop<0 || gParenStack[gParenTop]!=index) ){
		gParenTop++;
		gParenStack[gParenTop]= index;
		index++;
	}
		
	returnedAst = NULL;
	if( (k = expressionWithoutParenthese(index)) > index){
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
			//if(index > oldIndex && gParenTop>=0){
			//	gParenTop--;
			//}
					
			free(rs);
			return k;
		}
		sk = (NMASTList*)malloc(sizeof(NMASTList));
		sk->list = NULL;
		sk->loggedSize = 0;
		sk->size = 0;
		while(tk != NULL){
			if(isAnOperatorType(tk->type) 
					&& (  (l = expressionWithoutParenthese(k+1)) > (k+1) ) ){
				//Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type =  tk->type;
				op1->priority =  tk->priority;
				op1->sign =  1;
				op1->variable =  0;
				op1->parent = op1->left = op1->right;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				
				//add to pool
				pool[poolSize++] = op1;
				
				while( sk->size>0 && (sk->list[sk->size-1]->priority >= op1->priority )) {
					//pop
					op2 = popASTStack(sk);
					pushASTStack(rs, op2);
				}
				pushASTStack(sk, op1);
					
				//expressionWithoutParenthese
				pushASTStack(rs, returnedAst);
				k = l;
				tk = (k<gTokens->size)?gTokens->list[k]:NULL;
			}else if(tk->type == RPAREN){
				if(gParenTop < 0){
					//ERROR: got PRARENT but there is not any LPAREN in stack
					//TODO: need clear stack, clear rs, release stack, release rs
					for(k=0; k<poolSize; k++)
						free(pool[k]);
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
				while((l+1)<rs->size && !isAnOperatorType(itm->type)){
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
}//done
	
/**
 * expressionWithoutParenthese: LPAREN? parseExpressionElement (( + | - | * | / | ^) parseExpressionElement)* LPAREN?
 * TODO: fix this
 * @param index
 * @return
 */
int expressionWithoutParenthese(int index) {
	int k, l;
	int oldIndex = index;
	NMASTList *rs, *sk;
	NMAST *itm, *op1, *op2;
	
	NMAST	*pool[MAXPOOLSIZE];
	int poolSize = 0;
	
	Token *tk;
	
	if(index >= gTokens->size) return index;
	
	tk = gTokens->list[index];
	if(tk->type == LPAREN && ( gParenTop<0 || gParenStack[gParenTop] != index ) ){
		gParenTop++;
		gParenStack[gParenTop] = index;
		index++;
	}
		
	returnedAst = NULL;
	if( (k = expressionElement(index)) > index){
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
			//if(index > oldIndex && ( gParenTop>=0 )){
			//	gParenTop--;
			//}
					
			free(rs);
			return k;
		}
		sk = (NMASTList*)malloc(sizeof(NMASTList));
		sk->list = NULL;
		sk->loggedSize = 0;
		sk->size = 0;
		while(tk != NULL){
			if(isAnOperatorType(tk->type) 
					&& (  (l = expressionElement(k+1)) > (k+1) ) ){
				//Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type =  tk->type;
				op1->priority =  tk->priority;
				op1->sign =  1;
				op1->variable =  0;
				op1->parent = op1->left = op1->right;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				
				//add to pool
				pool[poolSize++] = op1;
				
				while( sk->size>0 && (sk->list[sk->size-1]->priority >= op1->priority )) {
					//pop
					op2 = popASTStack(sk);
					pushASTStack(rs, op2);
				}
				pushASTStack(sk, op1);
					
				//expressionElement
				pushASTStack(rs, returnedAst);
				k = l;
				tk = (k<gTokens->size)?gTokens->list[k]:NULL;
			}else if(tk->type == RPAREN) {
				if(gParenTop < 0){
					//ERROR:
					//TODO: need clear stack, clear rs, release stack, release rs
					for(k=0; k<poolSize; k++)
						free(pool[k]);
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
				while( (l+1)<rs->size && !isAnOperatorType(itm->type)){
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
	
/**
 * TODO: implement this routine
 * expressionElement: NUMBER | VARIABLE | CONSTANT | FUNCTION_CALL
 * @return
 */
int expressionElement(int index) {
	int k;
	Token *tk = gTokens->list[index];
	returnedAst = NULL;
	if(tk->type == NUMBER){
		returnedAst = (NMAST*)malloc(sizeof(NMAST));
		returnedAst->type = tk->type;
		returnedAst->value = parseDouble(tk->text, 0, tk->textLength, &gErrorCode);
		returnedAst->parent = returnedAst->left = returnedAst->right = NULL;
		returnedAst->variable = 0;
		return (index + 1);
	}else if(tk->type == PI_TYPE){
		returnedAst = (NMAST*)malloc(sizeof(NMAST));
		returnedAst->type = tk->type;
		returnedAst->value = PI;
		returnedAst->parent = returnedAst->left = returnedAst->right = NULL;
		returnedAst->variable = 0;
		return (index + 1);
	}else if(tk->type == E_TYPE){
		returnedAst = (NMAST*)malloc(sizeof(NMAST));
		returnedAst->type = tk->type;
		returnedAst->value = E;
		returnedAst->parent = returnedAst->left = returnedAst->right = NULL;
		returnedAst->variable = 0;
		return (index + 1);
	}else if(tk->type == NAME || tk->type == VARIABLE){
		returnedAst = (NMAST*)malloc(sizeof(NMAST));
		returnedAst->type = tk->type;
		returnedAst->value = 0;
		returnedAst->valueType = TYPE_FLOATING_POINT;
		returnedAst->variable = tk->text[0];
		returnedAst->parent = returnedAst->left = returnedAst->right = NULL;
		return (index + 1);
	}else if( (k = functionCall(index)) > index ){
		return k;
	}
	return index;
} //done
	
/**
 * functionCall: NAME LPAREN expression RPAREN;
 * @param index the start token index where we want to check if its a function call
 * @return index if its not a function call or the last token index of the function call
 */
int functionCall(int index){
	int k;
	NMAST* f;
	Token *tk;
	
	if(index >= gTokens->size)
		return index;
		
	tk = gTokens->list[index];
	gErrorCode = ERROR_BAD_TOKEN;
	gErrorColumn = gTokens->list[index]->column;
	if( /*(tk.getType() == Token.NAME) &&*/ isAFunctionType(tk->type)){
		gErrorCode = ERROR_PARENTHESE_MISSING;
		gErrorColumn = gTokens->list[index]->column;
		if( (index +1)<gTokens->size && gTokens->list[index + 1]->type == LPAREN ){
			if( gParenTop<0 || gParenStack[gParenTop] != (index+1)){
				gParenTop++;
				gParenStack[gParenTop] = index + 1;
			}
			
			f = (NMAST*) malloc(sizeof(NMAST));
			f->type = tk->type;
			f->parent = f->left = f->right = NULL;
			f->value = 0;
			f->valueType = TYPE_FLOATING_POINT;
			
			if((k=expression(index + 2)) > (index+2)){
				if(k<gTokens->size){
					gErrorCode = ERROR_PARENTHESE_MISSING;
					gErrorColumn = gTokens->list[k]->column;
					if(k<gTokens->size && gTokens->list[k]->type == RPAREN){
						if(gParenTop >= 0){
							//ERROR: missing open parenthese, PLEASE CHECK MEMORY HERE
							gErrorCode = ERROR_PARENTHESE_MISSING;
							gErrorColumn = gTokens->list[k]->column;
							free(f);
							clearTree(&returnedAst);
							returnedAst = NULL;
							return index;
						}
					}
				}
				gParenTop--;
				f->right = returnedAst;
				returnedAst = f;
				gErrorCode = NO_ERROR;
				gErrorColumn = -1;
				return k+1;
			}else{
				//ERROR: not an expression
				gErrorCode = ERROR_NOT_AN_EXPRESSION;
				gErrorColumn = gTokens->list[k]->column;
				free(f);
			}
		}
	}
	
	return index;
}//done
	
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
						tokenK5 = gTokens->list[currentIdx + 5];
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
		
	if(tk->type == NUMBER){
		tokenK1 = gTokens->list[idx + 1];
		if(isComparationOperator(tokenK1->type)){
			tokenK2 = gTokens->list[idx + 2];
			if(tokenK2->type == NAME || tokenK2->type == VARIABLE){
				tokenK3 = gTokens->list[idx + 3];
				if(isComparationOperator(tokenK3->type)){
					tokenK4 = gTokens->list[idx + 4];
					if(tokenK4->type == NUMBER){
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
	
	NMAST	*pool[MAXPOOLSIZE];
	int poolSize = 0;
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

				//add to pool
				pool[poolSize++] = op1;
				
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
					for(k=0; k<poolSize; k++)
						free(pool[k]);
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
	
//NOT use now, MUST fix before using
NMAST* parseDomain(TokenList *gTokens){
	int k;
	gParenTop = -1;
	if( (k=domain(currentIdx))>currentIdx){
		currentIdx = k;
		if(gParenTop==-1)
			return returnedAst;
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
	
	NMAST	*pool[MAXPOOLSIZE];
	int poolSize = 0;
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

				//Add to pool
				pool[poolSize++] = op1;
				
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
					for(k=0; k<poolSize; k++)
						free(pool[k]);
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