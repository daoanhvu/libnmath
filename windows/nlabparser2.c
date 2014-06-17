#include <stdlib.h>
#include <stdio.h>
#include "nlabparser2.h"

#define MAXPOOLSIZE 32

/** Global variables those are used in parsing */
int currentIdx = -1;
int errorIdx = -1;
int errorCode = 0;
int g_ParenInStack = 0;
Function *returnFunction = NULL;
NMAST *returnedAst = NULL;
TokenList *g_tokens = NULL;

/**
	access right: public
*/	
Function* parseFunctionExpression(TokenList *tokens){
	Function* fd = NULL;
	int k, l;
	if(tokens->list[currentIdx]->type == NAME && 
		(tokens->list[currentIdx + 1]->type==LPAREN)){
		if( (k=functionNotation(tokens, currentIdx + 2)) > (currentIdx + 2) ){	
			if(tokens->list[k]->type == EQ){
				k++;
				if( (l = expression(tokens, k))>k){
					returnFunction->prefix = (NMAST**)malloc(sizeof(NMAST*));
					returnFunction->prefixAllocLen = 1;
					returnFunction->prefixLen = 1;
					returnFunction->prefix[0] = returnedAst;

					if((k = domain(tokens, l))>l){
					}
				}
				
			} else {
				//Exception;
				errorIdx = currentToken.getColumn();
			}
		}
	}
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
int functionNotation(TokenList *tokens, int index){
	int i, k, varsize = 0;
	int oldIndex = index;
	Token *next = NULL;
	NMAST *functionDefNode = NULL;
	char vars[4];

	if(index >= tokens->size)
		return index;
	
	errorCode = ERROR_NOT_A_FUNCTION;
	if(tokens->list[index]->type == NAME ){
		errorCode = ERROR_PARENTHESE_MISSING;
		errorIdx = tokens->list[index]->column;
		if(tokens->list[index+1]->type == LPAREN){
			errorCode = ERROR_MISSING_VARIABLE;
			errorIdx = tokens->list[index+1]->column;
			if(tokens->list[index+2]->type == NAME){
				vars[varsize++] = (tokens->list[index+2])->text[0];
				index += 3;
				while( (index+1<tokens->size) && (tokens->list[index]->type == COMMA)
							&& (tokens->list[index+1]->type == NAME || tokens->list[index+1]->type == VARIABLE ) ){
					vars[varsize++] = (tokens->list[index+1])->text[0];
					index += 2;
				}
				errorCode = ERROR_PARENTHESE_MISSING;
				errorIdx = tokens->list[index]->column;
				if( (index<tokens->size) && (tokens->list[index]->type == RPAREN)){
					returnFunction = (Function*)malloc(sizeof(Function));
					//Should use memcpy here
					for(i=0;i<varsize;i++)
						returnFunction->variable[i] = vars[i];
						
					errorCode = NO_ERROR;
					errorIdx = -1;
					return (index + 1);
				}
			}
		}
	}
	errorIdx = tokens->list[index-2]->column;
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
int expression(TokenList *tokens, int index){
	int k, l, addedLParent = 0;
	int oldIndex = index;
	NMASTList *rs = (NMASTList*)malloc(sizeof(NMASTList));
	NMASTList *sk = (NMASTList*)malloc(sizeof(NMASTList));
	NMAST *itm, *op1, *op2;
	
	NMAST	*pool[MAXPOOLSIZE];
	int poolSize = 0;
	
	Token *tk = tokens->list[index];

	if(tk->type == LPAREN){
		g_ParenInStack++;
		addedLParent = 1;
		index++;
	}
		
	returnedAst = NULL;
	if( (k = expressionWithoutParenthese(index)) > index){
		pushASTStack(rs, returnedAst);
		if( k < tokens->size ) {
			tk = tokens->list[k];
		}else{
			//If we got a RPAREN here, maybe it belong to parent rule
			//TODO: need clear stack, clear rs, release stack, release rs
			return k;
		}
			
		while(tk != NULL){
			if(isAnOperatorType(tk->type) 
					&& (  (l = expressionWithoutParenthese(k+1)) > (k+1) ) ){
				//Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type =  tk->type;
				op1->sign =  1;
				op1->variable =  0;
				op1->parent = op1->left = op1->right;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				
				//add to pool
				pool[poolSize] = op1;
				
				while( sk->size>0 && (sk->list[sk->size-1]->priority >= op1->priority )) {
					//pop
					op2 = popASTStack(sk);
					pushASTStack(rs, op2);
				}
				pushASTStack(sk, op1);
					
				//expressionWithoutParenthese
				pushASTStack(rs, returnedAst);
				k = l;
				tk = tokens->list[k];
			}else if(tk->type == RPAREN){
				if(g_ParenInStack <= 0){
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
				g_ParenInStack--;
				k++;
				tk = NULL;
			}else{
				if(addedLParent)
					g_ParenInStack--;
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
				while(!isAnOperatorType(itm->type)){
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
	//TODO: need clear stack, clear rs, release stack, release rs
	for(k=0; k<poolSize; k++)
		free(pool[k]);
	free(sk);
	free(rs);
	return oldIndex;
}//done
	
/**
 * expressionWithoutParenthese: LPAREN? parseExpressionElement (( + | - | * | / | ^) parseExpressionElement)* LPAREN?
 * TODO: fix this
 * @param index
 * @return
 */
int expressionWithoutParenthese(TokenList *tokens, int index) {
	int k, l;
	int oldIndex = index;
	NMASTList *rs = (NMASTList*)malloc(sizeof(NMASTList));
	NMASTList *sk = (NMASTList*)malloc(sizeof(NMASTList));
	NMAST *itm, *op1, *op2;
	
	NMAST	*pool[MAXPOOLSIZE];
	int poolSize = 0;
	
	Token *tk = tokens->list[index];

	if(tk->type == LPAREN){
		g_ParenInStack++;
		index++;
	}
		
	returnedAst = NULL;
	if( (k = expressionElement(index)) > index){
		pushASTStack(rs, returnedAst);
		if( k < tokens->size ) {
			tk = tokens->list[k];
		}else{
			//If we got a RPAREN here, maybe it belong to parent rule
			//TODO: need clear stack, clear rs, release stack, release rs
			return k;
		}
			
		while(tk != NULL){
			if(isAnOperatorType(tk->type) 
					&& (  (l = expressionElement(k+1)) > (k+1) ) ){
				//Operator
				op1 = (NMAST*)malloc(sizeof(NMAST));
				op1->type =  tk->type;
				op1->sign =  1;
				op1->variable =  0;
				op1->parent = op1->left = op1->right;
				op1->value = 0.0;
				op1->valueType = TYPE_FLOATING_POINT;
				
				//add to pool
				pool[poolSize] = op1;
				
				while( sk->size>0 && (sk->list[sk->size-1]->priority >= op1->priority )) {
					//pop
					op2 = popASTStack(sk);
					pushASTStack(rs, op2);
				}
				pushASTStack(sk, op1);
					
				//expressionElement
				pushASTStack(rs, returnedAst);
				k = l;
				tk = tokens->list[k];
			}else if(tk->type == RPAREN) {
				if(g_ParenInStack <= 0){
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
				g_ParenInStack--;
				k++;
				tk = NULL;
			}else{
				if(index > oldIndex)
					g_ParenInStack--;

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
				while(!isAnOperatorType(itm->type)){
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
	//TODO: need clear stack, clear rs, release stack, release rs
	for(k=0; k<poolSize; k++)
		free(pool[k]);
	free(sk);
	free(rs);
	return oldIndex;
} //done
	
/**
 * TODO: implement this routine
 * expressionElement: NUMBER | VARIABLE | CONSTANT | FUNCTION_CALL
 * @return
 */
int expressionElement(TokenList *tokens, int index) {
	int k;
	Token *tk = tokens->list[index];
	returnedAst = NULL;
	if(tk->type == NUMBER){
		returnedAst = (NMAST*)malloc(sizeof(NMAST));
		returnedAst->type = tk->type;
		returnedAst->value = parseDouble(tk->text, 0, tk->testLength-1, &errorCode);
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
 * functionCall: NAME LPAREN expression RPARENT
 * @return
 */
NMAST* parseFunctionCall(TokenList *tokens){
	int k;
	g_ParenInStack = 0;
	if( (k=functionCall(currentIdx))>currentIdx){
		currentIdx = k+1;
		currentToken = NULL;
		if(currentIdx < tokens->size)
			currentToken = tokens->list[currentIdx];
		return returnedAst;
	}
	return NULL;
}//done
	
/**
 * functionCall: NAME LPAREN expression RPAREN;
 * @param index the start token index where we want to check if its a function call
 * @return index if its not a function call or the last token index of the function call
 */
int functionCall(TokenList *tokens, int index){
	int k;
	NMAST* f;
	Token *tk = tokens->list[index];
	errorCode = ERROR_BAD_TOKEN;
	errorIdx = tokens->list[index]->column;
	if( /*(tk.getType() == Token.NAME) &&*/ isAFunctionType(tk->type)){
		errorCode = ERROR_PARENTHESE_MISSING;
		errorIdx = tokens->list[index]->column;
		if( tokens->list[index + 1]->type == LPAREN ){
			if( (k=expression(tokens, index + 2)) > (index+2)){
				errorCode = ERROR_PARENTHESE_MISSING;
				errorIdx = tokens->list[k]->column;
				if(k<tokens->size && tokens->list[k]->type == RPAREN){
					errorCode = NO_ERROR;
					errorIdx = -1;
					return k+1;
				}
			}
		}
	}
	return index;
}//done
	
/**
 * interval: intervalWithBoundaries | intervalElementOf | simpleInterval;
 * @return the next index token after the interval
 */
int interval(TokenList *tokens, int idx){
	int nextIdx;
	if( (nextIdx = intervalWithBoundaries(idx)) > idx ){
		return nextIdx;
	}else if( (nextIdx = intervalElementOf(idx)) > idx){
		return nextIdx;
	}else if( (nextIdx = simpleInterval(tokens, idx)) > idx){
		return nextIdx;
	}
	return idx;
}//done
	
/**
 * simpleInterval : LPAREN? (NAME|VARIABLE) (LT | LTE | GT | GTE) (NUMBER|PI_TYPE|E_TYPE) RPARENT?
 * @param idx
 * @return
 */
int simpleInterval(TokenList *tokens, int idx){
	Token *tk = tokens->list[idx];
	int oldIdx = idx;
	
	if(tk->type == LPAREN){
		g_ParenInStack++;
		idx++;
		tk = tokens->list[idx];
	}
		
	if(tk->type == NAME || tk->type == VARIABLE){
		if(tokens->list[idx+1]->type==LT ||
				tokens->list[idx+1]->type==LTE || tokens->list[idx+1]->type==GT ||
				tokens->list[idx+1]->type==GTE) {
			if(tokens->list[idx+2]->type == NUMBER || tokens->list[idx+2]->type == PI_TYPE 
										|| tokens->list[idx+2]->type == E_TYPE) {
				if(tokens->list[idx + 3]->type == RPAREN){
					if(g_ParenInStack <= 0) {
						return oldIdx;
					}
					g_ParenInStack--;
					idx++;
				}else{
					if(idx > oldIdx) {
						g_ParenInStack--;
					}
				}
					
				returnedAst = (NMAST*)malloc(sizeof(NMAST));
				returnedAst->parent = NULL;
				returnedAst->type = tokens->list[idx+1]->type;
				returnedAst->variable = 0;
				returnedAst->value = 0;
				
				returnedAst->left = (NMAST*)malloc(sizeof(NMAST));
				returnedAst->left->parent = returnedAst;
				returnedAst->left->type = tk->type;
				returnedAst->left->variable = tk->text[0];
				returnedAst->left->value = 0;
				returnedAst->left->left = returnedAst->left->right = NULL;
				
				returnedAst->right = (NMAST*)malloc(sizeof(NMAST));
				returnedAst->right->parent = returnedAst;
				returnedAst->right->type = tokens->list[idx+2]->type;
				returnedAst->right->variable = 0;
				returnedAst->right->left = returnedAst->right->right = NULL;
				switch(tokens->list[idx+2]->type){
					case NUMBER:
						returnedAst->right->value = parseDouble(tokens->list[idx+2]->text, 0, tokens->list[idx+2]->textLength-1);
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
		
	return idx;
}//done
	
/**
 * intervalElementOf: NAME ELEMENT_OF (LPAREN | LPRACKET) NUMBER COMMA NUMBER (RPAREN | RPRACKET);
 * @param idx
 * @return
 */
int intervalElementOf(TokenList *tokens, int idx){
	double val1, val2;
	Token *tk = tokens->list[idx];
	Token *tokenK1, *tokenK2, *tokenK3, *tokenK4, *tokenK5, *tokenK6;
	returnedAst = NULL;
	if(tk->type == NAME){
		tokenK1 = tokens->list[idx + 1];
		if(tokenK1->type==ELEMENT_OF) {
			tokenK2 = tokens->list[idx + 2];
			if(tokenK2->type == LPAREN || tokenK2->type == LPRACKET ){
				tokenK3 = tokens->list[idx + 3];
				if(tokenK3->type == NUMBER || tokenK3->type == PI_TYPE || tokenK3->type == E_TYPE){
					switch(tokenK3->type){
						case NUMBER:
							val1 = parseDouble(tokenK3->text, 0, tokenK3->testLength-1, &errorIdx);
						break;

						case PI_TYPE:
							val1 = PI;
							break;

						case E_TYPE:
							val1 = E;
							break;
					}
					tokenK4 = tokens->list[idx + 4];
					if(tokenK4->type == COMMA){
						tokenK5 = tokens->list[currentIdx + 5];
						if(tokenK5->type == NUMBER){
							switch(tokenK5->type){
								case NUMBER:
									val2 = parseDouble(tokenK5->text, 0, tokenK5->testLength-1, &errorIdx);
								break;

								case PI_TYPE:
									val2 = PI;
									break;

								case E_TYPE:
									val2 = E;
									break;
							}
							tokenK6 = tokens->list[idx + 6];
							if(tokenK6->type == RPAREN || tokenK6->type == RPRACKET ){
								returnedAst = (NMAST*)malloc(sizeof(NMAST));
								returnedAst->type = GT_LT;
								returnedAst->variable = tk->text[0];
								returnedAst->parent = NULL;

								returnedAst->left = (NMAST*)malloc(sizeof(NMAST));
								returnedAst->left->type = NUMBER;
								returnedAst->left->value = val1;
								returnedAst->left->parent = returnedAst;

								returnedAst->right = (NMAST*)malloc(sizeof(NMAST));
								returnedAst->right->type = NUMBER;
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
 * intervalWithLowerAndUpper: LPAREN? NUMBER (LT | LTE | GT | GTE) NAME (LT | LTE | GT | GTE) NUMBER RPARENT? ;
 * @return
 */
int intervalWithBoundaries(TokenList *tokens, int idx){
	Token *tk = tokens->list[idx];
	Token *tokenK1, *tokenK2, *tokenK3, *tokenK4;
	String op1Text="", textTypeName = "";
	int oldIdx = idx;

	if(tk->type == LPAREN){
		g_ParenInStack++;
		idx++;
		tk = tokens->list[idx];
	}
		
	if(tk->type == NUMBER){
		tokenK1 = tokens->list[idx + 1];
		if(isComparationOperator(tokenK1->type)){
			tokenK2 = tokens->list[idx + 2];
			if(tokenK2->type == NAME){
				tokenK3 = tokens->list[idx + 3];
				if(isComparationOperator(tokenK3->type)){
					tokenK4 = tokens->list[idx + 4];
					if(tokenK4->type == NUMBER){
						if(tokens->list[idx + 5]->type == RPAREN){
							if(g_ParenInStack <= 0)
								return oldIdx;

							g_ParenInStack--;
							idx++;
						}else{
							if(idx > oldIdx) {
								g_ParenInStack--;
							}
						}
							
						/**
							The reason for doing this is that we always
							read the comparation in the order that 
							variable comes first.
							Example: -1 < x < 3
								x is greater than -1 and x is less than 3
						*/
						if("LT".equals(tokenK1.getTypeName()))
							op1Text = "GT";
						else if("GT".equals(tokenK1.getTypeName()))
							op1Text = "LT";
						else if("LTE".equals(tokenK1.getTypeName()))
							op1Text = "GTE";
						else if("GTE".equals(tokenK1.getTypeName()))
							op1Text = "LTE";
							
						textTypeName = op1Text + "_" + tokenK3.getTypeName();
						returnedAst = new AST(AbstractAST.INTERVAL, textTypeName, tk.getColumn());
						AST nameNode = new AST(tokenK2.getType(), tokenK2.getText(), tokenK2.getColumn());
						returnedAst.addChild(nameNode);
						AST lBound = new AST(tk.getType(), tk.getText(), tk.getColumn());
						returnedAst.addChild(lBound);
						AST rBound = new AST(tokenK4.getType(), tokenK4.getText(), tokenK4.getColumn());
						returnedAst.addChild(rBound);
						
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
	List<AST> rs = new ArrayList<AST>();
	Stack<AST> sk = new Stack<AST>();
	AST itm, op1, op2;
	Token tk = tokens.get(index);
		
		if(tk.getType() == Token.LPAREN){
			stackCheckParen.push(tk);
			index++;
		}
		
		returnedAst = NULL;
		if( (k=interval(index)) > index ){
			rs.add(returnedAst);
			if( k < tokens.size() ){
				tk = tokens.get(k);
			}else{
				return k;
			}
			
			while( tk != null ){
				if(((op1 = parseOR_AND()) != null) && ((l = interval(k+1))>(k+1))) {
					while((sk.peek().getType()==AbstractAST.OR||sk.peek().getType()==AbstractAST.AND)
							&&(sk.peek().getPriority() >= op1.getPriority())){
						op2 = sk.pop();
						rs.add(op2);
					}
					sk.push(op1);
					
					rs.add(returnedAst);
					k = l;
					tk = tokens.get(k);
				}else if(tk.getType() == Token.RPAREN){
					/**
					 * it's is possible that this RPAREN comes from parent rule
					 */
					if(stackCheckParen.isEmpty()){
						return oldIndex;
					}
					stackCheckParen.pop();
					k++;
					tk = null;
				}else{
					tk = null;
				}
			}
			
			while(!sk.isEmpty()){
				rs.add(sk.pop());
			}
			
			if(rs.size() > 0){
				//Build tree from infix
				while(rs.size() > 1){
					l = 2;
					itm = rs.get(l);
					while(itm.getType() != AbstractAST.OR && itm.getType() != AbstractAST.AND){
						l++;
						itm = rs.get(l);
					}
					AST operand1 = rs.get(l-2);
					AST operand2 = rs.get(l-1);
					rs.remove(operand1);
					rs.remove(operand2);
					itm.addChild(operand1);
					itm.addChild(operand2);
				}
				if(rs.get(0).getType() != AbstractAST.INTERVAL){
					if(rs.get(0).getChildrent().size()==1)
						returnedAst = rs.get(0).getChildrent().get(0);
					else{
						returnedAst = new AST(AbstractAST.DOMAIN, "DOMAIN",rs.get(0).getColumn());
						returnedAst.addChild(rs.get(0));
					}
				}else
					returnedAst = rs.get(0);
			}
			
			return k;
		}
		return oldIndex;
	}
	
	private AST parseOR_AND() {
		if(currentToken.getType() == Token.OR || 
				currentToken.getType() == Token.AND){
			AST ast = new AST(currentToken.getType(), 
									currentToken.getText(), currentToken.getColumn());
			currentIdx++;
			currentToken = null;
			if(currentIdx < tokens.size())
				currentToken = tokens.get(currentIdx);
			return ast;
		}
		return null;
	}
	
NMAST* parseDomain(){
	int k;
	stackCheckParen.clear();
	if( (k=domain(currentIdx))>currentIdx){
		currentIdx = k;
		currentToken = null;
		if(currentIdx < tokens.size())
			currentToken = tokens.get(currentIdx);
		if(stackCheckParen.isEmpty())
			return returnedAst;
	}
	return null;
}

/**
 * domain: simple_domain ( (OR | AND) simple_domain)* ;
 */
int domain(TokenList *tokens, int index){
	int k, l;
	int oldIndex = index;
	List<AST> rs = new ArrayList<AST>();
	Stack<AST> sk = new Stack<AST>();
	AST itm, op1, op2;
	Token tk = tokens.get(index);
	
	if(tk.getType() == Token.LPAREN){
		stackCheckParen.push(tk);
		index++;
	}
		
	returnedAst = null;
	if( (k=simpleDomain(index)) > index ){
		rs.add(returnedAst);
		if( k < tokens.size() ){
			tk = tokens.get(k);
		}else{
			return k;
		}
			
		while( tk != null ){
			if((tk.getType()==AbstractAST.OR||tk.getType()==AbstractAST.AND) && ((l = simpleDomain(k+1))>(k+1))) {
				while(!sk.isEmpty() && (sk.peek().getType()==AbstractAST.OR||sk.peek().getType()==AbstractAST.AND)
						&&(sk.peek().getPriority() >= tk.getPriority())){
					op2 = sk.pop();
					rs.add(op2);
				}
				sk.push(new AST(tk.getType(), tk));
				
				rs.add(returnedAst);
				k = l;
				tk = (k<tokens.size())?tokens.get(k):null;
			}else if(tk.getType() == Token.RPAREN){
				/**
				 * it's is possible that this RPAREN comes from parent rule
				 */
				if(stackCheckParen.isEmpty())
					return oldIndex; //<< missing LPAREN
				stackCheckParen.pop();
				k++;
				tk = null;
			}else{
				tk = null;
			}
		}
			
		while(!sk.isEmpty()){
			rs.add(sk.pop());
		}
			
		if(rs.size() > 0){
			//Build tree from infix
			while(rs.size() > 1){
				l = 2;
				itm = rs.get(l);
				while(itm.getType() != AbstractAST.OR && itm.getType() != AbstractAST.AND){
					l++;
					itm = rs.get(l);
				}
				AST operand1 = rs.get(l-2);
				AST operand2 = rs.get(l-1);
				rs.remove(operand1);
				rs.remove(operand2);
				itm.addChild(operand1);
				itm.addChild(operand2);
			}
			if(rs.get(0).getChildrent()!=null && rs.get(0).getChildrent().size()>0){
				if(rs.get(0).getChildrent().size()==1)
					returnedAst = rs.get(0).getChildrent().get(0);
				else{
					returnedAst = new AST(AbstractAST.DOMAIN, "DOMAIN",rs.get(0).getColumn());
					returnedAst.addChild(rs.get(0));
				}
			}else
				returnedAst = rs.get(0);
		}
			
		return k;
	}
	return oldIndex;
}

	/**
	 *	A function notation is like f(x, y)
	 *
	 */
	public AST functionNotation(){
		int i, k;
		Token next = null;
		AST functionDefNode = null;
		
		for(i=currentIdx; i<tokens.size();i++){
			if( tokens.get(i).getType() == Token.EQ )
				break;
		}
		
		functionDefNode = new AST(Token.FUNCTION_NOTATION, currentToken);
		currentIdx++;
		currentToken = null;
		if(currentIdx < tokens.size())
			currentToken = tokens.get(currentIdx);
		else return null;
		
		if(currentToken.getType() == Token.LPAREN){
			currentIdx++;
			currentToken = tokens.get(currentIdx);
			if(currentToken.getType() == Token.VARIABLE || currentToken.getType() == Token.NAME){
				AST v = new AST(Token.VARIABLE, currentToken);
				functionDefNode.addChild(v);
				
				currentIdx++;
				currentToken = tokens.get(currentIdx);
				k = currentIdx + 1;
				next = tokens.get(k);
				while(true){
					if( (currentToken.getType() == Token.COMMA) && 
												( next.getType() == Token.VARIABLE || next.getType() == Token.NAME)	) {
						v = new AST(Token.VARIABLE, next);
						functionDefNode.addChild(v);
						
						currentIdx += 2;
						currentToken = tokens.get(currentIdx);
						k = currentIdx + 1;
						next = tokens.get(k);
					} else{
						if(currentToken.getType() == Token.RPAREN){
							currentIdx++;
							currentToken = tokens.get(currentIdx);
						}else{
							//Exception
							error = currentToken.getColumn();
						}
					
						return functionDefNode;
					}
				}
			}
		}
		
		return functionDefNode;
	}
	
	private boolean checkParenthese(int from, int to){
		int i;
		Token tk;
		stackCheckParen.clear();		
		for(i = from; i<=to; i++){
			tk = tokens.get(i);
			if(tk.getType() == Token.LPAREN)
				stackCheckParen.push(tk);
			else if(tk.getType() == Token.RPAREN){
				if(stackCheckParen.isEmpty() ||
						stackCheckParen.peek().getType() != Token.LPAREN) 
					return false; // << missing LPAREN
				stackCheckParen.pop();
			}
		}
		return stackCheckParen.isEmpty()?true:false;
	}

	public int getError() {
		return error;
	}
	
	public boolean hasError() {
		return error<0?false:true;
	}