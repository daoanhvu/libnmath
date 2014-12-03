#include <stdlib.h>

#ifdef DEBUG
	#include <stdio.h>
#endif

#include "StackUtil.h"
#include "nlablexer.h"
#include "nlabparser.h"


#ifdef _TARGET_HOST_ANDROID
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

int functionNotation(const TokenList *tokens, int index, char *variables, int *variableCount);
NMAST* domain(int *start, TokenList *tokens);
NMAST* buildIntervalTree(Token* valtk1, Token* o1, Token* variable, Token* o2, Token* valtk2);

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

void addFunction2Tree(NMASTList *t, Token * stItm){
	NMAST *ast = NULL;
	// LOGI(2, "Type: %d (%s)", stItm->type, stItm->text);
	switch(stItm->type) {
		case PLUS:
			if(t->size > 1) {
				ast = getFromPool();
				ast->type = stItm->type;
				ast->priority = stItm->priority;
				ast->left = t->list[t->size-2];
				ast->right = t->list[t->size-1];
				if((t->list[t->size-2])!=NULL)
					(t->list[t->size-2])->parent = ast;
				if((t->list[t->size-1])!=NULL)
					(t->list[t->size-1])->parent = ast;
					
				t->list[t->size-2] = ast;
				t->list[t->size-1] = NULL;
				t->size--;
			}
		break;

		case MINUS:
			if(t->size == 1) {
				if((t->list[0]) != NULL)
					(t->list[0])->sign = -1;
			} else {
				ast = getFromPool();
				ast->type = stItm->type;
				ast->priority = stItm->priority;
				ast->left = t->list[t->size-2];
				ast->right = t->list[t->size-1];
				if((t->list[t->size-2])!=NULL)
					(t->list[t->size-2])->parent = ast;
				if((t->list[t->size-1])!=NULL)
					(t->list[t->size-1])->parent = ast;
					
				t->list[t->size-2] = ast;
				t->list[t->size-1] = NULL;
				t->size--;
			}
		break;

		case MULTIPLY:
		case DIVIDE:
		case POWER:
		
		case LT:
		case GT:
		case LTE:
		case GTE:
		case AND:
		case OR:
			ast = getFromPool();
			ast->type = stItm->type;
			ast->priority = stItm->priority;
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
			ast = getFromPool();
			ast->type = stItm->type;
			ast->priority = stItm->priority;
			ast->right = t->list[t->size-1];
			if((t->list[t->size-1])!=NULL)
				(t->list[t->size-1])->parent = ast;
				
			t->list[t->size-1] = ast;
		break;
										
		case LOG:
			ast = getFromPool();
			ast->type = stItm->type;
			ast->priority = stItm->priority;
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
	@return errorCode
*/	
int NLabParser::parseFunctionExpression(NLabLexer& lexer) {
	int k, l, i, idx = 0;
	errorCode = ERROR_NOT_A_FUNCTION;
	errorColumn = lexer[idx]->column;
	
	// LOGI(3, "[NativeParser] GOT HERE - token size: %d", tokens->size);
	/** This array will hold the variables of the function */
	mVarCount = 0;
	if ((k = functionNotation(lexer, idx)) > idx){
		if(lexer[k]->type == EQ){

			mPrefix = NULL;
			mDomain = NULL;

			for(i=0; i<lexer.size(); i++) {
				if(lexer[i]->type == NAME) {
					for(l=0; l<mVarCount; l++) {
						if(lexer[i]->text[0]==mVariables[l] && lexer[i]->textLength==1)
							lexer[i]->type = VARIABLE;
					}
				}
			}
			
			k++;
			do {
				/*
					Parse expression
				*/
				parseExpression(lexer, &k);
				/** after parseExpression, we may get error, so MUST check if it's OK here */
				if( (errorCode!=NMATH_NO_ERROR) || (k >= lexer.size()) ) break;
				
				if(lexer[k]->type == DOMAIN_NOTATION) {
					errorCode = ERROR_MISSING_DOMAIN;
					errorColumn = lexer[k]->column;
					if(k+1 < lexer.size()) {
						l = k + 1;
						parseDomain(lexer, &l);
						k = l;
					}
				}
			} while ( errorCode==NMATH_NO_ERROR && k < lexer.size() );
		}
	} else {
		/*
			The input tokens not started with a function notation then
			we try to parse expression here.
		*/
		//reset errorCode
		errorCode = NMATH_NO_ERROR;

		mPrefix = NULL;
		mDomain = NULL;
		k = 0;
		parseExpression(lexer, &k);

	}
	
	if(errorCode != NMATH_NO_ERROR){
		if(mPrefix != NULL) {
			for(k=0; k<mPrefix->size; k++){
				clearTree(&(mPrefix->list[k]));
			}
			mPrefix->size = 0;
			free(mPrefix);
		}
			
		if(mDomain != NULL) {
			for(k=0; k<mDomain->size; k++){
				clearTree(&(mDomain->list[k]));
			}
			mDomain->size = 0;
			free(mDomain);
		}
		
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
int NLabParser::functionNotation(NLabLexer& lexer, int index) {
	int oldIndex = index;

	if( (index < 0) || index >= lexer.size())
		return index;
		
	mVarCount = 0;
	errorCode = ERROR_NOT_A_FUNCTION;
	errorColumn = lexer[index]->column;
	if(lexer[index]->type == NAME ) {
		errorCode = ERROR_PARENTHESE_MISSING;
		if(lexer[index+1]->type == LPAREN) {
			errorCode = ERROR_MISSING_VARIABLE;
			errorColumn = lexer[index+1]->column;
			
			if(lexer[index+2]->type == NAME){
				mVariables[mVarCount++] = lexer[index+2]->text[0];
				index += 3;
				while( (index+1<lexer.size()) && (lexer[index]->type == COMMA)
							&& (lexer[index+1]->type == NAME ) ){
					mVariables[mVarCount++] = lexer[index+1]->text[0];
					index += 2;
				}
				errorCode = ERROR_PARENTHESE_MISSING;
				errorColumn = lexer[index]->column;
				if( (index < lexer.size()) && (lexer[index]->type == RPAREN)){
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
void NLabParser::parseExpression(NLabLexer& lexer, int *start) {
	int i, top=-1, allocLen=0, isEndExp = FALSE;
	int error;
	double val;
	int size = lexer.size();
	Token *tk = NULL;
	Token **stack = NULL;
	Token *stItm = NULL;
	NMAST *ast = NULL;
	//NMAST* varNodes[50];
	mPrefix = (NMASTList*)malloc(sizeof(NMASTList));
	mPrefix->size = 0;
	mPrefix->loggedSize = 0;
	mPrefix->list = NULL;

	errorColumn = -1;
	errorCode = NMATH_NO_ERROR;
	
	i = (*start);
	// LOGI(2, "[parseExpression] Before while loop i=%d", i);
	while( (i < size) && !isEndExp) {
		tk = lexer[i];
		// LOGI(2, "token %d type:%d", i, tk->type);
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
					return;
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
					return;
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
					return;
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
					return ;
				}

				/*  */
				while(stItm!=NULL && (stItm->type != LPAREN) && isAFunctionType(stItm->type)  != TRUE){
					addFunction2Tree(mPrefix, stItm);
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
					return;
				}

				if(isAFunctionType(stItm->type)  == TRUE){
					addFunction2Tree(mPrefix, stItm);
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
					return;
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
				ast->type = tk->type;
				ast->variable = tk->text[0];
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
				return;
		}//end switch
	}//end while

	while(top >= 0){
		stItm = StackUtil::popFromStack(stack, &top);
		
		if(stItm->type == LPAREN || isAFunctionType(stItm->type)==TRUE){
			clearStackWithoutFreeItem(stack, top+1);
			free(stack);
			for (i = 0; i<mPrefix->size; i++)
				clearTree(&(mPrefix->list[i]));
			free(mPrefix->list);
			free(mPrefix);
			errorColumn = tk->column;
			errorCode = ERROR_PARENTHESE_MISSING;
			return; 
		}
		addFunction2Tree(mPrefix, stItm);
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

int NLabParser::parseDomain(NLabLexer& lexer, int *start) {
	int isEndExp = FALSE;
	int i, index, top = -1, allocLen=0;
	Token* tk;
	double val, val2;
	Token **stack = NULL;
	Token *tokenItm = NULL;
	NMAST *ast;

	mDomain = (NMASTList*)malloc(sizeof(NMASTList));
	mDomain->size = 0;
	mDomain->loggedSize = 0;
	mDomain->list = NULL;
	
	errorColumn = -1;
	errorCode = NMATH_NO_ERROR;
	
	index = *start;
	while(index < lexer.size() && !isEndExp){
		tk = lexer[index];
		switch(tk->type) {
			case NUMBER:
			case PI_TYPE:
			case E_TYPE:
				if( (index+4)<lexer.size() && isComparationOperator(lexer[index+1]->type) 
									&& lexer[index+2]->type==VARIABLE 
									&& isComparationOperator(lexer[index+3]->type)
									&& (lexer[index+4]->type==NUMBER || lexer[index+4]->type==PI_TYPE || lexer[index+4]->type==E_TYPE )) {
					/**
						HERE, I missed the case that NUMBER < VARIABLE < NUMBER or
						NUMBER <= VARIABLE < NUMBER or NUMBER < VARIABLE <= NUMBER or 
						NUMBER <= VARIABLE <= NUMBER	
						
						Build an AND tree to hold the case
					*/
					ast = buildIntervalTree(tk, lexer[index+1], lexer[index+2], lexer[index+3], lexer[index+4]);
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
					addFunction2Tree(mDomain, tokenItm);
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
					addFunction2Tree(mDomain, tokenItm);
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
					errorColumn = lexer[index]->column;
					return NULL;
				}
				index++;
			break;
			
			case VARIABLE:
				if(( (index+1) < lexer.size()) && lexer[index+1]->type == ELEMENT_OF){
					/*
						
						VARIABLE ELEMENT_OF [NUMBER,NUMBER]
						VARIABLE ELEMENT_OF (NUMBER,NUMBER]
						VARIABLE ELEMENT_OF [NUMBER,NUMBER)
						VARIABLE ELEMENT_OF (NUMBER,NUMBER)
					*/
					if( ( index+6 < lexer.size()) && (lexer[index+2]->type == LPRACKET || lexer[index+2]->type == LPAREN)
								&& (lexer[index+3]->type == NUMBER || lexer[index+3]->type == PI_TYPE || lexer[index+3]->type == E_TYPE)
								&& lexer[index+4]->type == COMMA 
								&& (lexer[index + 5]->type == NUMBER || lexer[index + 5]->type == PI_TYPE || lexer[index + 5]->type == E_TYPE)
								&& (lexer[index + 6]->type == RPRACKET || lexer[index + 6]->type == RPAREN)) {
								
						
						/** ========START Parse floating point values======= */
						switch(lexer[index+3]->type) {
							case NUMBER:
								val = parseDouble(lexer[index+3]->text, 0, lexer[index+3]->textLength, &errorCode);
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
						
						switch(lexer[index+5]->type){
							case NUMBER:
								val2 = parseDouble(lexer[index + 5]->text, 0, lexer[index + 5]->textLength, &errorCode);
								if(val2 == 0 && errorCode != NMATH_NO_ERROR){
									clearStackWithoutFreeItem(stack, top+1);
									free(stack);
									for (i = 0; i<mDomain->size; i++)
										clearTree(&(mDomain->list[i]));
									free(mDomain->list);
									free(mDomain);
									errorColumn = lexer[index + 4]->column;
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
						if ((lexer[index + 2]->type == LPAREN) && (lexer[index + 6]->type == RPAREN))
							ast->type = GT_LT;
						else if ((lexer[index + 2]->type == LPRACKET) && (lexer[index + 6]->type == RPAREN))
							ast->type = GTE_LT;
						else if ((lexer[index + 2]->type == LPAREN) && (lexer[index + 6]->type == RPRACKET))
							ast->type = GT_LTE;
						else if ((lexer[index + 2]->type == LPRACKET) && (lexer[index + 6]->type == RPRACKET))
							ast->type = GTE_LTE;
						
						//ast->Left number 1
						ast->left = getFromPool();
						ast->left->parent = ast;
						ast->left->value = val;
						ast->left->type = lexer[index + 3]->type;
						//Left->Right NUMBER or PI_TYPE or E_TYPE
						ast->right = getFromPool();
						ast->right->parent = ast;
						ast->right->value = val2;
						ast->right->type = lexer[index + 5]->type;
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
						return ERROR_SYNTAX;
					}
				}else {
					ast = getFromPool();
					ast->variable = tk->text[0];
					ast->value = val;
					ast->type = tk->type;
					pushASTStack(mDomain, ast);
					index++;
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
			return ERROR_PARENTHESE_MISSING;
		}
		addFunction2Tree(mDomain, tokenItm);
	}
	*start = index;
	free(stack);

	return NMATH_NO_ERROR;
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
	ast = (NMAST*)malloc(sizeof(NMAST));
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