#include <stdlib.h>

#ifdef DEBUG
	#include <stdio.h>
#endif

#include "StackUtil.h"
#include "nlablexer.h"
#include "nlabparser.h"
#include "nmath_pool.h"

#ifdef _ADEBUG
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NLABPARSER"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif



using namespace nmath;

//internal variables
NMAST *returnedAst = NULL;

NLabParser::NLabParser(){
}

NLabParser::~NLabParser() {
}

int NLabParser::getType(const Token *t) {
	int i;
	for(i=0; i<mVarCount; i++) {
#ifdef _ADEBUG
		LOGI(2, "token_text=%s, token_text_length=%d, variable=%c, VARIABLE=%d", t->text, t->textLength, mVariables[i], VARIABLE);
#endif
		if((t->text[0] == mVariables[i]) && (t->textLength == 1)) {
			return VARIABLE;
		}
	}
	return t->type;
}

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

int NLabParser::parseFunctionExpression(Token* tokens, int tokenCount, NMASTList *prefix, NMASTList *domain) {
	int k, l, i, idx = 0;
	errorCode = ERROR_NOT_A_FUNCTION;
	errorColumn = tokens[idx].column;
	int oldOffset;

	NMAST *item;
	NMAST **tempList;
	
	//LOGI(3, "[NativeParser] GOT HERE - token size: %d", tokens->size);
	/** This array will hold the variables of the function */
	mVarCount = 0;
	if ((k = functionNotation(tokens, tokenCount, idx)) > idx){
		if(tokens[k].type == EQ){

			/*
			for(i=0; i<tokenCount; i++) {
				if(tokens[i].type == NAME) {
					for(l=0; l<mVarCount; l++) {
						if(tokens[i].text[0]==mVariables[l] && tokens[i].textLength==1)
							tokens[i].type = VARIABLE;
							LOGI(3, "Token %d change to Variable (type=%d)", i, tokens[i].type);
					}
				}
				LOGI(3, "Type of token %d = %d", i, tokens[i].type);
			}
			*/
			
			k++;
			do {
				/*
					Parse expression
				*/
				item = parseExpression(tokens, tokenCount, &k);
				/** after parseExpression, we may get error, so MUST check if it's OK here */
				if( errorCode!=NMATH_NO_ERROR ) break;

				if(prefix->size >= prefix->loggedSize) {
					prefix->loggedSize += 1;
					tempList = (NMAST**)realloc(prefix->list, sizeof(NMAST*) * prefix->loggedSize);
					if(tempList != NULL) {
						prefix->list = tempList;
						prefix->list[prefix->size++] = item;
					}

					oldOffset = domain->loggedSize;
					domain->loggedSize = prefix->loggedSize;
					tempList = (NMAST**)realloc(domain->list, sizeof(NMAST*) * domain->loggedSize);
					if(tempList != NULL) {
						domain->list = tempList;
						memset(domain->list + oldOffset, 0, domain->loggedSize - oldOffset);

						//domain->list[domain->loggedSize-1] = NULL;
					}
				}

				item = 0;
				if( (k < tokenCount) && (tokens[k].type == DOMAIN_NOTATION) ) {
					errorCode = ERROR_MISSING_DOMAIN;
					errorColumn = tokens[k].column;
					if(k+1 < tokenCount) {
						l = k + 1;
						item = parseDomain(tokens, tokenCount, &l);
						//domain->list[domain->size++] = item;
						k = l;
					}
				}

				domain->list[domain->size++] = item;

			} while ( errorCode==NMATH_NO_ERROR && k < tokenCount );
		}
	}
	
	if(errorCode != NMATH_NO_ERROR){
		for(k=0; k<prefix->size; k++){
			clearTree(&(prefix->list[k]));

			if(domain->list[k] != NULL){
				clearTree(&(domain->list[k]));
			}
		}
		prefix->size = 0;
		prefix->loggedSize = 0;
		free(prefix->list);

		domain->size = 0;
		domain->loggedSize = 0;
		free(domain->list);
		
		if(returnedAst != NULL){
			clearTree(&returnedAst);
			returnedAst = NULL;
		}	
	}
	return errorCode;
}

/**
	functionNotation: NAME LPAREN NAME (COMA NAME)* PRARENT;
	@return if
*/
int NLabParser::functionNotation(Token* tokens, int tokenCount, int index) {
	int oldIndex = index;

	if( (index < 0) || index >= tokenCount )
		return index;
		
	mVarCount = 0;
	errorCode = ERROR_NOT_A_FUNCTION;
	errorColumn = tokens[index].column;
	if(tokens[index].type == NAME ) {
		errorCode = ERROR_PARENTHESE_MISSING;
		if(tokens[index+1].type == LPAREN) {
			errorCode = ERROR_MISSING_VARIABLE;
			errorColumn = tokens[index+1].column;
			
			if(tokens[index+2].type == NAME){
				mVariables[mVarCount++] = tokens[index+2].text[0];
#ifdef _ADEBUG
		LOGI(2, "variable(%d)=%c", mVarCount-1, mVariables[mVarCount-1]);
#endif
				index += 3;
				while( (index+1<tokenCount) && (tokens[index].type == COMMA)
							&& (tokens[index+1].type == NAME ) ){
					mVariables[mVarCount++] = tokens[index+1].text[0];
					index += 2;
				}
				errorCode = ERROR_PARENTHESE_MISSING;
				errorColumn = tokens[index].column;
				if( (index < tokenCount) && (tokens[index].type == RPAREN)){
					errorCode = NMATH_NO_ERROR;
					errorColumn = -1;
					return (index + 1);
				}
			}
		}
	}
	return oldIndex;
}//done

/******************************************************************************************/
/**
	Parse the input string in object f to NMAST tree
*/
NMAST* NLabParser::parseExpression(Token* tokens, int size, int *start) {
	int i, top=-1, allocLen=0, isEndExp = FALSE;
	int error;
	double val;
	Token *tk = NULL;
	Token **stack = NULL;
	Token *stItm = NULL;
	NMAST *ast = NULL;
	NMASTList* mPrefix = (NMASTList*)malloc(sizeof(NMASTList));
	mPrefix->size = 0;
	mPrefix->loggedSize = 0;
	mPrefix->list = NULL;

	errorColumn = -1;
	errorCode = NMATH_NO_ERROR;
	
	i = (*start);
	// LOGI(2, "[parseExpression] Before while loop i=%d", i);
	while( (i < size) && !isEndExp) {
		tk = &(tokens[i]);
		switch(tk->type) {
			case NUMBER:
				val = parseDouble(tk->text, 0, tk->textLength, &error);
				if(val == 0 && error < 0) {
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mPrefix->size; i++)
						clearTree(&(mPrefix->list[i]));
					free(mPrefix->list);
					free(mPrefix);
					errorColumn = tk->column;
					errorCode = ERROR_PARSING_NUMBER;
					return NULL;
				}

				ast = getFromPool();
				ast->value = val;
				ast->type = tk->type;
				pushASTStack(mPrefix, ast); //add this item to prefix
				i++;
				break;

			case E_TYPE:
				ast = getFromPool();
				ast->value = E;
				ast->type = E_TYPE;
				pushASTStack(mPrefix, ast); //add this item to prefix
				i++;
				break;

			case PI_TYPE:
				ast = getFromPool();
				ast->value = PI;
				ast->type = PI_TYPE;
				pushASTStack(mPrefix, ast); //add this item to prefix
				i++;
				break;
				

			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
			case POWER:
				if(top >= 0){
					stItm = stack[top];
					// LOGI(2, "Token: type = %d, text=%s", tk->type, tk->text);
					while((isAnOperatorType(stItm->type)==TRUE) && (stItm->priority) >= tk->priority){
						stItm = StackUtil::popFromStack(stack, &top);

						//LOGI(2, "Token just popped from Stack: type = %d, text=%s, PREFIX SIZE= %d", stItm->type, stItm->text, prefix->size);

						if (mPrefix->size == 1) {
							mPrefix->list[0]->sign = -1;
						} else {
							ast = getFromPool();
							ast->type = stItm->type;
							ast->priority = stItm->priority;
							ast->left = mPrefix->list[mPrefix->size - 2];
							ast->right = mPrefix->list[mPrefix->size - 1];
							
							if((ast->left)!=NULL)
								(ast->left)->parent = ast;
							if((ast->right)!=NULL)
								(ast->right)->parent = ast;

							mPrefix->list[mPrefix->size - 2] = ast;
							mPrefix->list[mPrefix->size - 1] = NULL;
							mPrefix->size--;
						}

						if(top < 0)
							break;

						stItm = stack[top];
					}
				}
				//push operation o1 (tk) into stack
				StackUtil::pushItem2Stack(&stack, &top, &allocLen, tk);
				if(errorCode == E_NOT_ENOUGH_MEMORY) {
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mPrefix->size; i++)
						clearTree(&(mPrefix->list[i]));
					free(mPrefix->list);
					free(mPrefix);
					errorColumn = tk->column;
					return NULL;
				}
				i++;
				break;

			case LPAREN:/*If it an open parentheses then put it to stack*/
				StackUtil::pushItem2Stack(&stack, &top, &allocLen, tk);
				if(errorCode == E_NOT_ENOUGH_MEMORY){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mPrefix->size; i++)
						clearTree(&(mPrefix->list[i]));
					free(mPrefix->list);
					free(mPrefix);
					errorColumn = tk->column;
					return NULL;
				}
				i++;
				break;

			case RPAREN:
				stItm = StackUtil::popFromStack(stack, &top);

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(stItm == NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mPrefix->size; i++)
						clearTree(&(mPrefix->list[i]));
					free(mPrefix->list);
					free(mPrefix);
					errorColumn = tk->column;
					errorCode = ERROR_PARENTHESE_MISSING;
					return NULL;
				}

				/*  */
				while(stItm!=NULL && (stItm->type != LPAREN) && isAFunctionType(stItm->type)  != TRUE){
					StackUtil::addFunction2Tree(mPrefix, stItm);
					//free(stItm);
					stItm = StackUtil::popFromStack(stack, &top);
				}

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(stItm==NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mPrefix->size; i++)
						clearTree(&(mPrefix->list[i]));
					free(mPrefix->list);
					free(mPrefix);
					errorColumn = tk->column;
					errorCode = ERROR_PARENTHESE_MISSING;
					return NULL;
				}

				if(isAFunctionType(stItm->type)  == TRUE){
					StackUtil::addFunction2Tree(mPrefix, stItm);
				}
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
			
			/*
				if( (i+2)>=tokens->size || tokens->list[i+1].type != LPAREN){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					for(i=0;i<prefix->size;i++)
						clearTree(&(prefix->list[i]));
					free(prefix->list);
					free(prefix);
#ifdef DEBUG
	descNumberOfDynamicObject();
	descNumberOfDynamicObject();
#endif
					errorColumn = tk->column;
					errorCode = ERROR_PARENTHESE_MISSING;
					return;
				}
			*/
				
				StackUtil::pushItem2Stack(&stack, &top, &allocLen, tk);
				if(errorCode == E_NOT_ENOUGH_MEMORY){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mPrefix->size; i++)
						clearTree(&(mPrefix->list[i]));
					free(mPrefix->list);
					free(mPrefix);
					errorColumn = tk->column;
					return NULL;
				}

				/**
					After a function name must be a LPAREN, and we just ignore that LPAREN token
					i += 2;
				*/

				// the left-parenthese right after a function name is dimissed, so just don't care of it
				i++;

				break;

			case NAME:
			case VARIABLE:
				ast = getFromPool();
				//ast->type = tk->type;
				ast->type = getType(tk);
				ast->variable = tk->text[0];
#ifdef _ADEBUG
		//LOGI(2, "Node (type=%d, variable=%c, sign=%d, value=%lf)", ast->type, ast->variable, ast->sign, ast->value);
#endif
				pushASTStack(mPrefix, ast); //add this item to prefix

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
				for (i = 0; i<mPrefix->size; i++)
					clearTree(&(mPrefix->list[i]));
				free(mPrefix->list);
				free(mPrefix);
				errorColumn = tk->column;
				errorCode = ERROR_BAD_TOKEN;
				//LOGI(2, "[parseExpression] Error at Token_index=%d; Token_type=%d; ErrorCode=%d; ErrorColumn=%d", i, tk->type, errorCode, errorColumn);
				return NULL;
		}//end switch
	}//end while

	while(top >= 0){
		stItm = StackUtil::popFromStack(stack, &top);
		
		if(stItm->type == LPAREN || isAFunctionType(stItm->type)==TRUE) {
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			for (i = 0; i<mPrefix->size; i++)
				clearTree(&(mPrefix->list[i]));
			free(mPrefix->list);
			free(mPrefix);
			errorColumn = tk->column;
			errorCode = ERROR_PARENTHESE_MISSING;
			return NULL;
		}
		StackUtil::addFunction2Tree(mPrefix, stItm);
	}
	
	free(stack);
	*start = i;
	
	//if(f->numVarNode > 0) {
	//	f->variableNode = (NMAST**)malloc(sizeof(NMAST*) * f->numVarNode);
	//	for(i=0; i<f->numVarNode; i++){
	//		f->variableNode[i] = varNodes[i];
	//	}
	//}
	ast = mPrefix->list[0];

	free(mPrefix->list);
	free(mPrefix);

	return ast;
}

NMAST* NLabParser::parseDomain(Token *tokens, int tokenCount, int *start) {
	int isEndExp = FALSE;
	int i, index, top = -1, allocLen=0;
	Token* tk;
	double val, val2;
	Token **stack = NULL;
	Token *tokenItm = NULL;
	NMAST *ast;

	NMASTList* mDomain = (NMASTList*)malloc(sizeof(NMASTList));
	mDomain->size = 0;
	mDomain->loggedSize = 0;
	mDomain->list = NULL;
	
	errorColumn = -1;
	errorCode = NMATH_NO_ERROR;
	
	index = *start;
	while( (index < tokenCount) && !isEndExp) {
		tk = &(tokens[index]);
		switch(tk->type) {
			case NUMBER:
			case PI_TYPE:
			case E_TYPE:
				if( (index+4)<tokenCount && isComparationOperator(tokens[index+1].type) 
									&& tokens[index+2].type==VARIABLE 
									&& isComparationOperator(tokens[index+3].type)
									&& (tokens[index+4].type==NUMBER || tokens[index+4].type==PI_TYPE || tokens[index+4].type==E_TYPE )) {
					/**
						HERE, I missed the case that NUMBER < VARIABLE < NUMBER or
						NUMBER <= VARIABLE < NUMBER or NUMBER < VARIABLE <= NUMBER or 
						NUMBER <= VARIABLE <= NUMBER	
						
						Build an AND tree to hold the case
					*/
					ast = buildIntervalTree(tk, &tokens[index+1], &tokens[index+2], &tokens[index+3], &tokens[index+4]);
					if (ast == NULL) {
						clearStackWithoutFreeItem(stack, top+1);
						free(stack);
						for (i = 0; i<mDomain->size; i++)
							clearTree(&(mDomain->list[i]));
						free(mDomain->list);
						free(mDomain);
						return NULL;
					}
					pushASTStack(mDomain, ast);
					index += 5;
				}// end if
				
				switch(tk->type){
					case NUMBER:
						val = parseDouble(tk->text, 0, tk->textLength, &errorCode);
						if(val == 0 && errorCode != NMATH_NO_ERROR){
							clearStackWithoutFreeItem(stack, top+1);
							free(stack);
							for (i = 0; i<mDomain->size; i++)
								clearTree(&(mDomain->list[i]));
							free(mDomain->list);
							free(mDomain);
							errorColumn = tk->column;
							return NULL;
						}
						break;
					case PI_TYPE:
							val = PI;
						break;
					case E_TYPE:
							val = E;
						break;
				}

				ast = getFromPool();
				ast->value = val;
				ast->type = tk->type;
				pushASTStack(mDomain, ast);
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
					while((isAnOperatorType(tokenItm->type)==TRUE || isComparationOperator(tokenItm->type)==TRUE || tokenItm->type==AND || tokenItm->type==OR)
								&& (tokenItm->priority) >= tk->priority){
						tokenItm = StackUtil::popFromStack(stack, &top);

						ast = getFromPool();
						ast->type = tokenItm->type;
						ast->priority = tokenItm->priority;
						ast->left = mDomain->list[mDomain->size - 2];
						ast->right = mDomain->list[mDomain->size - 1];
						if((ast->type == LT || ast->type == LTE || ast->type == GT || ast->type == GTE )
							&& (mDomain->list[mDomain->size - 1]->type == VARIABLE)){
							ast->left = mDomain->list[mDomain->size - 1];
							ast->right = mDomain->list[mDomain->size - 2];
							
							switch(ast->type){
								case LT:
									ast->type = GT;
								break;
								
								case LTE:
									ast->type = GTE;
								break;
								
								case GT:
									ast->type = LT;
								break;
								
								case GTE:
									ast->type = LTE;
								break;
							}
							
							
						}
						
						if((ast->left)!=NULL)
							(ast->left)->parent = ast;
						if((ast->right)!=NULL)
							(ast->right)->parent = ast;
						
						mDomain->list[mDomain->size - 2] = ast;
						mDomain->list[mDomain->size - 1] = NULL;
						mDomain->size--;
						
						if(top < 0)
							break;

						tokenItm = stack[top];
					}
				}
				//push operation o1 (tk) into stack
				StackUtil::pushItem2Stack(&stack, &top, &allocLen, tk);
				if(errorCode == E_NOT_ENOUGH_MEMORY){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mDomain->size; i++)
						clearTree(&(mDomain->list[i]));
					free(mDomain->list);
					free(mDomain);
					errorColumn = tk->column;
					return NULL;
				}
				index++;
			break;

			case RPAREN:
				tokenItm = StackUtil::popFromStack(stack, &top);

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(tokenItm == NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mDomain->size; i++)
						clearTree(&(mDomain->list[i]));
					free(mDomain->list);
					free(mDomain);
					errorColumn = tk->column;
					errorCode = ERROR_PARENTHESE_MISSING;
					return NULL;
				}

				/*  */
				while(tokenItm!=NULL && (tokenItm->type != LPAREN) && isAFunctionType(tokenItm->type)  != TRUE){
					StackUtil::addFunction2Tree(mDomain, tokenItm);
					tokenItm = StackUtil::popFromStack(stack, &top);
				}

				if(tokenItm==NULL){
					/** ERROR: got an opening-parenthese but can not find a closing-parenthese */
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mDomain->size; i++)
						clearTree(&(mDomain->list[i]));
					free(mDomain->list);
					free(mDomain);
					errorColumn = tk->column;
					errorCode = ERROR_PARENTHESE_MISSING;
					return NULL;
				}

				if(isAFunctionType(tokenItm->type)  == TRUE){
					StackUtil::addFunction2Tree(mDomain, tokenItm);
				}
				
				index++;
			break;
				
			case LPAREN:
				StackUtil::pushItem2Stack(&stack, &top, &allocLen, tk);
				if(errorCode == E_NOT_ENOUGH_MEMORY){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
					for (i = 0; i<mDomain->size; i++)
						clearTree(&(mDomain->list[i]));
					free(mDomain->list);
					free(mDomain);
					errorColumn = tokens[index].column;
					return NULL;
				}
				index++;
			break;
			
			case NAME:
			case VARIABLE:
				if(( (index+1) < tokenCount) && tokens[index+1].type == ELEMENT_OF){
					/*
						
						VARIABLE ELEMENT_OF [NUMBER,NUMBER]
						VARIABLE ELEMENT_OF (NUMBER,NUMBER]
						VARIABLE ELEMENT_OF [NUMBER,NUMBER)
						VARIABLE ELEMENT_OF (NUMBER,NUMBER)
					*/
					if( ( index+6 < tokenCount) && (tokens[index+2].type == LPRACKET || tokens[index+2].type == LPAREN)
								&& (tokens[index+3].type == NUMBER || tokens[index+3].type == PI_TYPE || tokens[index+3].type == E_TYPE)
								&& tokens[index+4].type == COMMA 
								&& (tokens[index + 5].type == NUMBER || tokens[index + 5].type == PI_TYPE || tokens[index + 5].type == E_TYPE)
								&& (tokens[index + 6].type == RPRACKET || tokens[index + 6].type == RPAREN)) {
								
						
						/** ========START Parse floating point values======= */
						switch(tokens[index+3].type) {
							case NUMBER:
								val = parseDouble(tokens[index+3].text, 0, tokens[index+3].textLength, &errorCode);
								if(val == 0 && errorCode != NMATH_NO_ERROR){
									clearStackWithoutFreeItem(stack, top+1);
									free(stack);
									for(i=0;i<mDomain->size;i++)
										clearTree(&(mDomain->list[i]));
									free(mDomain->list);
									free(mDomain);
									errorColumn = tk->column;
									return NULL;
								}
							break;
							
							case PI_TYPE:
								val = PI;
							break;
							case E_TYPE:
								val = E;
							break;
						}
						
						switch(tokens[index+5].type){
							case NUMBER:
								val2 = parseDouble(tokens[index + 5].text, 0, tokens[index + 5].textLength, &errorCode);
								if(val2 == 0 && errorCode != NMATH_NO_ERROR){
									clearStackWithoutFreeItem(stack, top+1);
									free(stack);
									for (i = 0; i<mDomain->size; i++)
										clearTree(&(mDomain->list[i]));
									free(mDomain->list);
									free(mDomain);
									errorColumn = tokens[index + 4].column;
									return NULL;
								}
							break;
							
							case PI_TYPE:
								val2 = PI;
							break;
							case E_TYPE:
								val2 = E;
							break;
						}
						/** ========END parsing floating point values=====*/
						
						/**
							This case, x element_of [number1, number2]
							We parse to a tree that have root got:
								- type GT_LT or GTE_LT or GT_TLE or GTE_LTE and
								- variable = x
						*/
						ast = getFromPool();
						ast->variable = tk->text[0];
						if ((tokens[index + 2].type == LPAREN) && (tokens[index + 6].type == RPAREN))
							ast->type = GT_LT;
						else if ((tokens[index + 2].type == LPRACKET) && (tokens[index + 6].type == RPAREN))
							ast->type = GTE_LT;
						else if ((tokens[index + 2].type == LPAREN) && (tokens[index + 6].type == RPRACKET))
							ast->type = GT_LTE;
						else if ((tokens[index + 2].type == LPRACKET) && (tokens[index + 6].type == RPRACKET))
							ast->type = GTE_LTE;
						
						//ast->Left number 1
						ast->left = getFromPool();
						ast->left->parent = ast;
						ast->left->value = val;
						ast->left->type = tokens[index + 3].type;
						//Left->Right NUMBER or PI_TYPE or E_TYPE
						ast->right = getFromPool();
						ast->right->parent = ast;
						ast->right->value = val2;
						ast->right->type = tokens[index + 5].type;
						pushASTStack(mDomain, ast);
						index += 7;		
					}else{
						/**
							ERROR:After ELEMENT_OF is not a right syntax
						*/
						clearStackWithoutFreeItem(stack, top+1);
						free(stack);
						for (i = 0; i<mDomain->size; i++)
							clearTree(&(mDomain->list[i]));
						free(mDomain->list);
						free(mDomain);
						errorColumn = tk->column;
						errorCode = ERROR_SYNTAX;
						return NULL;
					}
				}else {
					// VARIABLE OPERATOR VALUE
					if(isComparationOperator(tokens[index+1].type)) {
						if( isConstant(tokens[index+2].type) ) {
						
							//OPERATOR
							ast = getFromPool();
							ast->type = tokens[index + 1].type;
							ast->variable = tk->text[0]; //Variable or NAME is stored at operator node
							ast->value = parseDouble(tokens[index+2].text, 0, tokens[index+2].textLength, &errorCode);
							
							pushASTStack(mDomain, ast);
							index += 3;
						}
					} else {
						ast = getFromPool();
						ast->variable = tk->text[0];
						ast->value = 0;
						ast->type = tk->type;
						pushASTStack(mDomain, ast);
						index++;
					}
				}
			break;
			
			case SEMI:
				isEndExp = TRUE;
			break;
		}
	}
	while(top >= 0){
		tokenItm = StackUtil::popFromStack(stack, &top);
		
		if(tokenItm->type == LPAREN || isAFunctionType(tokenItm->type)==TRUE){
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			for (i = 0; i<mDomain->size; i++)
				clearTree(&(mDomain->list[i]));
			free(mDomain->list);
			free(mDomain);
			errorColumn = tk->column;
			errorCode = ERROR_PARENTHESE_MISSING;
			return NULL;
		}
		StackUtil::addFunction2Tree(mDomain, tokenItm);
	}
	*start = index;
	free(stack);

	ast = mDomain->list[0];

	free(mDomain->list);
	free(mDomain);

	return ast;
}

NMAST* NLabParser::buildIntervalTree(Token* valtk1, Token* o1, Token* variable, Token* o2, Token* valtk2) {
	NMAST* ast = NULL;
	NMAST *valNode1, *valNode2;
	double val1, val2;
	int type, isSwap = FALSE;
	
	/** ERROR cases: -3 < x > 3 or -3 > x < 3  */
	if( ((o1->type == LT || o1->type == LTE) && (o2->type == GT || o2->type == GTE))
				|| ((o1->type == GT || o1->type == GTE) && (o2->type == LT || o2->type == LTE)) ) {
		errorCode = ERROR_SYNTAX;
		errorColumn = o2->column;
		return NULL;
	}
	
	/** ======================================================================== */
	switch(valtk1->type){
		case NUMBER:
			val1 = parseDouble(valtk1->text, 0, valtk1->textLength, &errorCode);
			if(val1 == 0 && errorCode != NMATH_NO_ERROR){
				errorColumn = valtk1->column;
				return NULL;
			}
		break;
						
		case PI_TYPE:
			val1 = PI;
		break;
		case E_TYPE:
			val1 = E;
		break;
	}
					
	switch(valtk2->type){
		case NUMBER:
			val2 = parseDouble(valtk2->text, 0, valtk2->textLength, &errorCode);
			if(val2 == 0 && errorCode != NMATH_NO_ERROR){
				errorColumn = valtk2->column;
				return NULL;
			}
		break;
						
		case PI_TYPE:
			val2 = PI;
		break;
		case E_TYPE:
			val2 = E;
		break;
	}
	
	valNode1 = (NMAST*)malloc(sizeof(NMAST));
	valNode1->variable = 0;
	valNode1->type = valtk1->type;
	valNode1->value = val1;
	valNode1->valueType = TYPE_FLOATING_POINT;
	valNode1->left = valNode1->right = NULL;
	valNode2 = (NMAST*)malloc(sizeof(NMAST));
	valNode2->variable = 0;
	valNode2->type = valtk2->type;
	valNode2->value = val2;
	valNode2->valueType = TYPE_FLOATING_POINT;
	valNode2->left = valNode2->right = NULL;
	
	/** ================================================================ */
	if((o1->type == LT ) && (o2->type == LT))
		type = GT_LT;
	else if((o1->type == LTE ) && (o2->type == LT))
		type = GTE_LT;
	else if((o1->type == LT ) && (o2->type == LTE))
		type = GT_LTE;
	else if((o1->type == LTE ) && (o2->type == LTE))
		type = GTE_LTE;
	else{
		isSwap = TRUE;
		if((o1->type == GT ) && (o2->type == GT))
			type = GT_LT;
		else if((o1->type == GTE ) && (o2->type == GT))
			type = GT_LTE;
		else if((o1->type == GT ) && (o2->type == GTE))
			type = GTE_LT;
		else if((o1->type == GTE ) && (o2->type == GTE))
			type = GTE_LTE;
	}

	//ast: GTE_LT | GT_TL
	ast = getFromPool();
	ast->type = type;
	ast->priority = 0;
	ast->variable = variable->text[0];
	ast->parent = NULL;
	if(isSwap){
		ast->left = valNode2;
		valNode2->parent = ast;
		
		ast->right = valNode1;
		valNode1->parent = ast;
	}else{
		ast->left = valNode1;
		valNode1->parent = ast;
		
		ast->right = valNode2;
		valNode2->parent = ast;
	}
	return ast;
}