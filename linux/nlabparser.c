#include <stdlib.h>

#ifdef DEBUG
	#include <stdio.h>
#endif

#include "nlablexer.h"
#include "common.h"
#include "nlabparser.h"

/**
#ifdef _TARGET_HOST_ANDROID
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NMATH2"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif
*/

extern short gErrorColumn;
extern short gErrorCode;
extern TokenList *gTokens;

//internal variables
NMAST *returnedAst = NULL;

int functionNotation(int index, char *vars, int *varCount);
void domain(int *start, Function *f);
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

void pushItem2Stack(Token ***st, int *top, int *allocLen, Token *item){
	Token** tmp;
	if( (*top) >= ( (*allocLen)-1)){
		(*allocLen) += INCLEN;
		tmp = (Token**)realloc(*st, sizeof(Token*) * (*allocLen) );
		if(tmp == NULL){
			gErrorCode = E_NOT_ENOUGH_MEMORY;
			return;
		}
		(*st) = tmp;
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
		
		case LT:
		case GT:
		case LTE:
		case GTE:
		case AND:
		case OR:
			ast = (NMAST*)malloc(sizeof(NMAST));
#ifdef DEBUG
			incNumberOfDynamicObject();
#endif
			ast->valueType = TYPE_FLOATING_POINT;
			ast->sign = 1;
			ast->type = stItm->type;
			ast->variable = 0;
			ast->priority = stItm->priority;
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
#ifdef DEBUG
			incNumberOfDynamicObject();
#endif
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
#ifdef DEBUG
			incNumberOfDynamicObject();
#endif
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
	access right: private
	@param outF a NOT-NULL Function pointer
	@return errorCode
*/	
int parseFunctionExpression(TokenList *tokens, Function *outF){
	int k, l, i, idx = 0;
	char variables[8];
	int variableCount = 0;
	
	gTokens = tokens;
	gErrorCode = ERROR_NOT_A_FUNCTION;
	gErrorColumn = tokens->list[idx]->column;
	
	/** This array will hold the variables of the function */	
	if( (k=functionNotation(idx, variables, &variableCount)) > idx ){
		if(tokens->list[k]->type == EQ){

			outF->prefix = NULL;
			outF->domain = NULL;
			outF->str = NULL;
			outF->len = 0;
			outF->variableNode = NULL;
			outF->numVarNode = 0;
			outF->valLen = variableCount;
			for(i=0;i<variableCount; i++) //Should use memcpy here
				outF->variable[i] = variables[i];

			for(i=0; i<tokens->size; i++){
				if(tokens->list[i]->type == NAME){
					for(l=0; l<variableCount; l++){
						if(tokens->list[i]->text[0]==variables[l] 
										&& tokens->list[i]->textLength==1)
							tokens->list[i]->type = VARIABLE;
					}
				}
			}
			
			k++;
			do{
				/*
					Parse expression
				*/
				parseExpression(tokens, &k, outF);
				/** after parseExpression, we may get error, so MUST check if it's OK here */
				if( (gErrorCode!=NMATH_NO_ERROR) || (k >= tokens->size) ) break;
				
				if(tokens->list[k]->type == DOMAIN_NOTATION){
					gErrorCode = ERROR_MISSING_DOMAIN;
					gErrorColumn = tokens->list[k]->column;
					if(k+1 < tokens->size){
						l = k + 1;
						domain(&l, outF);
						k = l;
					}
				}
			}while( gErrorCode==NMATH_NO_ERROR && k < tokens->size );
		}
	} else {
		/*
			The input tokens not started with a function notation then
			we try to parse expression here.
		*/
		//reset errorCode
		gErrorCode = NMATH_NO_ERROR;

		outF->prefix = NULL;
		outF->domain = NULL;
		outF->str = NULL;
		outF->len = 0;
		outF->variableNode = NULL;
		outF->numVarNode = 0;
		outF->valLen = 0;
		k = 0;
		parseExpression(tokens, &k, outF);

	}
	
	if(gErrorCode != NMATH_NO_ERROR){
		if(outF->prefix != NULL) {
			for(k=0; k<outF->prefix->size; k++){
				clearTree(&(outF->prefix->list[k]));
			}
			outF->prefix->size = 0;
			free(outF->prefix);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
		}
			
		if(outF->domain != NULL) {
			for(k=0; k<outF->domain->size; k++){
				clearTree(&(outF->domain->list[k]));
			}
			outF->domain->size = 0;
			free(outF->domain);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
		}
		
		if(returnedAst != NULL){
			clearTree(&returnedAst);
			returnedAst = NULL;
		}	
	}
	gTokens = NULL;
	return gErrorCode;
}

/**
	functionNotation: NAME LPAREN NAME (COMA NAME)* PRARENT;
	@return if
*/
int functionNotation(int index, char *variables, int *variableCount){
	int oldIndex = index;

	if( (index < 0) || index >= gTokens->size)
		return index;
		
	*variableCount = 0;
	gErrorCode = ERROR_NOT_A_FUNCTION;
	gErrorColumn = gTokens->list[index]->column;
	if(gTokens->list[index]->type == NAME ){
		gErrorCode = ERROR_PARENTHESE_MISSING;
		if(gTokens->list[index+1]->type == LPAREN){
			gErrorCode = ERROR_MISSING_VARIABLE;
			gErrorColumn = gTokens->list[index+1]->column;
			
			if(gTokens->list[index+2]->type == NAME){
				variables[(*variableCount)++] = (gTokens->list[index+2])->text[0];
				index += 3;
				while( (index+1<gTokens->size) && (gTokens->list[index]->type == COMMA)
							&& (gTokens->list[index+1]->type == NAME ) ){
					variables[(*variableCount)++] = (gTokens->list[index+1])->text[0];
					index += 2;
				}
				gErrorCode = ERROR_PARENTHESE_MISSING;
				gErrorColumn = gTokens->list[index]->column;
				if( (index<gTokens->size) && (gTokens->list[index]->type == RPAREN)){
					gErrorCode = NMATH_NO_ERROR;
					gErrorColumn = -1;
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
void parseExpression(TokenList *tokens, int *start, Function *f){
	int i, top=-1, allocLen=0, isEndExp = FALSE;
	short error;
	DATA_TYPE_FP val;
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
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	prefix->size = 0;
	prefix->loggedSize = 0;
	prefix->list = NULL;
	
	gErrorColumn = -1;
	gErrorCode = NMATH_NO_ERROR;
	
	f->numVarNode = 0;
	i = (*start);
	while(i < tokens->size && !isEndExp){
		tk = tokens->list[i];
		switch(tk->type){
			case NUMBER:
				val = parseFloatingPoint(tk->text, 0, tk->textLength, &error);
				if(val == 0 && error < 0){
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
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARSING_NUMBER;
					return;
				}

				ast = (NMAST*)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				ast->valueType = TYPE_FLOATING_POINT;
				ast->sign = 1;
				ast->left = ast->right = ast->parent = NULL;
				ast->value = val;
				ast->variable = 0;
				ast->type = tk->type;
				pushASTStack(prefix, ast); //add this item to prefix
				i++;
				break;

			case E_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				ast->left = ast->right = ast->parent = NULL;
				ast->valueType = 0;
				ast->value = E;
				ast->variable = 0;
				ast->type = E_TYPE;
				pushASTStack(prefix, ast); //add this item to prefix
				i++;
				break;

			case PI_TYPE:
				ast = (NMAST*)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				ast->left = ast->right = ast->parent = NULL;
				ast->value = PI;
				ast->type = PI_TYPE;
				ast->valueType = 0;
				ast->variable = 0;
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
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						ast->left = ast->right = NULL;
						ast->type = stItm->type;
						ast->variable = 0;
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
				if(gErrorCode == E_NOT_ENOUGH_MEMORY){
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
	descNumberOfDynamicObjectBy(2);
#endif
					gErrorColumn = tk->column;
					return;
				}
				i++;
				break;

			case LPAREN:/*If it an open parentheses then put it to stack*/
				pushItem2Stack(&stack, &top, &allocLen, tk);
				if(gErrorCode == E_NOT_ENOUGH_MEMORY){
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
	descNumberOfDynamicObjectBy(2);
#endif
					gErrorColumn = tk->column;
					return;
				}
				i++;
				break;

			case RPAREN:
				stItm = popFromStack(stack, &top);

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(stItm == NULL){
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
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return;
				}

				if(isAFunctionType(stItm->type)  == TRUE){
					addFunction2Tree(prefix, stItm);
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
			
				if( (i+2)>=tokens->size || tokens->list[i+1]->type != LPAREN){
					//After function name token is not a LPAREN
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
					gErrorColumn = tk->column;
					gErrorCode = ERROR_PARENTHESE_MISSING;
					return;
				}
				
				pushItem2Stack(&stack, &top, &allocLen, tk);
				if(gErrorCode == E_NOT_ENOUGH_MEMORY){
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
	descNumberOfDynamicObjectBy(2);
#endif
					gErrorColumn = tk->column;
					return;
				}
				/**
					After a function name must be a LPAREN, and we just ignore that LPAREN token
				*/
				i += 2;
				break;

			case NAME:
			case VARIABLE:
				ast = (NMAST*)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				ast->parent = ast->left = ast->right = NULL;
				ast->type = tk->type;
				ast->variable = tk->text[0];
				ast->value = 0;
				ast->sign = 1;
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
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
				for(i=0;i<prefix->size;i++)
					clearTree(&(prefix->list[i]));
				free(prefix->list);
				free(prefix);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
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
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
			for(i=0;i<prefix->size;i++)
				clearTree(&(prefix->list[i]));
			free(prefix->list);
			free(prefix);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
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
	if(f->prefix == NULL){
		f->prefix = (NMASTList*)malloc(sizeof(NMASTList));
		f->prefix->list = NULL;
		f->prefix->loggedSize = 0;
		f->prefix->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	}
		
	pushASTStack(f->prefix, prefix->list[0]);
	
	free(stack);
	free(prefix);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
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
short parseFunction(const char *str, short len, Function *outF){
	TokenList lst;
	int i, result;
	
	lst.loggedSize = 10;
	lst.list = (Token**)malloc(sizeof(Token*) * lst.loggedSize);
	lst.size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif

	/* build the tokens list from the input string */
	parseTokens(str, len, &lst);

#ifdef DEBUG
	printf("\n[NLabParser] Number of dynamic objects after parsing tokens: %d \n", numberOfDynamicObject() );
#endif
	
	if( gErrorCode == NMATH_NO_ERROR ){
		result = parseFunctionExpression(&lst, outF);
		for(i = 0; i<lst.size; i++){
			free(lst.list[i]);
		}
#ifdef DEBUG
	descNumberOfDynamicObjectBy(lst.size + 1);
#endif
	}
	free(lst.list);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
	return result;
}

void domain(int *start, Function *f){
	int isEndExp = FALSE;
	int i, index, top = -1, allocLen=0;
	Token* tk;
	DATA_TYPE_FP val, val2;
	Token **stack = NULL;
	NMASTList *d;
	Token *tokenItm = NULL;
	NMAST *ast;

	if(gTokens == NULL){
		gErrorColumn = 0;
		gErrorCode = ERROR_BAD_TOKEN;
		return ;
	}
	d = (NMASTList*)malloc(sizeof(NMASTList));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	d->size = 0;
	d->loggedSize = 0;
	d->list = NULL;
	
	gErrorColumn = -1;
	gErrorCode = NMATH_NO_ERROR;
	
	index = *start;
	while(index < gTokens->size && !isEndExp){
		tk = gTokens->list[index];
		switch(tk->type){
			case NUMBER:
			case PI_TYPE:
			case E_TYPE:
				if( (index+4)<gTokens->size && isComparationOperator(gTokens->list[index+1]->type) 
									&& gTokens->list[index+2]->type==VARIABLE 
									&& isComparationOperator(gTokens->list[index+3]->type)
									&& (gTokens->list[index+4]->type==NUMBER || gTokens->list[index+4]->type==PI_TYPE || gTokens->list[index+4]->type==E_TYPE )) {
					/**
						HERE, I missed the case that NUMBER < VARIABLE < NUMBER or
						NUMBER <= VARIABLE < NUMBER or NUMBER < VARIABLE <= NUMBER or 
						NUMBER <= VARIABLE <= NUMBER	
						
						Build an AND tree to hold the case
					*/
					ast = buildIntervalTree(tk, gTokens->list[index+1], gTokens->list[index+2], gTokens->list[index+3], gTokens->list[index+4]);
					if(tokenItm == NULL){
						clearStackWithoutFreeItem(stack, top+1);
						free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
						for(i=0;i<d->size;i++)
							clearTree(&(d->list[i]));
						free(d->list);
						free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
						return ;
					}
					pushASTStack(d, ast);
					index += 5;
				}// end if
				
/*
				Line 824 below is also have a malloc, why?
				ast = (NMAST*)malloc(sizeof(NMAST));
				

#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
*/
				switch(tk->type){
					case NUMBER:
						val = parseFloatingPoint(tk->text, 0, tk->textLength, &gErrorCode);
						if(val == 0 && gErrorCode != NMATH_NO_ERROR){
							clearStackWithoutFreeItem(stack, top+1);
							free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
							for(i=0;i<d->size;i++)
								clearTree(&(d->list[i]));
							free(d->list);
							free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
							gErrorColumn = tk->column;
							return;
						}
						break;
						case PI_TYPE:
							val = PI;
						break;
						case E_TYPE:
							val = E;
						break;
				}

				ast = (NMAST*)malloc(sizeof(NMAST));
				ast->variable = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
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
					while((isAnOperatorType(tokenItm->type)==TRUE || isComparationOperator(tokenItm->type)==TRUE || tokenItm->type==AND || tokenItm->type==OR)
								&& (tokenItm->priority) >= tk->priority){
						tokenItm = popFromStack(stack, &top);

						ast = (NMAST*)malloc(sizeof(NMAST));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						ast->variable = 0;
						ast->type = tokenItm->type;
						ast->priority = tokenItm->priority;
						ast->left = d->list[d->size-2];
						ast->right = d->list[d->size-1];
						if((ast->type == LT || ast->type == LTE || ast->type == GT || ast->type == GTE )
									&& (d->list[d->size-1]->type==VARIABLE) ){
							ast->left = d->list[d->size-1];
							ast->right = d->list[d->size-2];
							
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
				if(gErrorCode == E_NOT_ENOUGH_MEMORY){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					gErrorColumn = tk->column;
					return;
				}
				index++;
			break;

			case RPAREN:
				tokenItm = popFromStack(stack, &top);

				/* got an opening-parenthese but can not find a closing-parenthese */
				if(tokenItm == NULL){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
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
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
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
				if(gErrorCode == E_NOT_ENOUGH_MEMORY){
					clearStackWithoutFreeItem(stack, top+1);
					free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					for(i=0;i<d->size;i++)
						clearTree(&(d->list[i]));
					free(d->list);
					free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					gErrorColumn = gTokens->list[index]->column;
					return;
				}
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
								
						
						/** ========START Parse floating point values======= */
						switch(gTokens->list[index+3]->type){
							case NUMBER:
								val = parseFloatingPoint(gTokens->list[index+3]->text, 0, gTokens->list[index+3]->textLength, &gErrorCode);
								if(val == 0 && gErrorCode != NMATH_NO_ERROR){
									clearStackWithoutFreeItem(stack, top+1);
									free(stack);
	#ifdef DEBUG
		descNumberOfDynamicObject();
	#endif
									for(i=0;i<d->size;i++)
										clearTree(&(d->list[i]));
									free(d->list);
									free(d);
	#ifdef DEBUG
		descNumberOfDynamicObjectBy(2);
	#endif
									gErrorColumn = tk->column;
									return;
								}
							break;
							
							case PI_TYPE:
								val = PI;
							break;
							case E_TYPE:
								val = E;
							break;
						}
						
						switch(gTokens->list[index+5]->type){
							case NUMBER:
								val2 = parseFloatingPoint(gTokens->list[index+5]->text, 0, gTokens->list[index+5]->textLength, &gErrorCode);
								if(val2 == 0 && gErrorCode != NMATH_NO_ERROR){
									clearStackWithoutFreeItem(stack, top+1);
									free(stack);
	#ifdef DEBUG
		descNumberOfDynamicObject();
	#endif
									for(i=0;i<d->size;i++)
										clearTree(&(d->list[i]));
									free(d->list);
									free(d);
	#ifdef DEBUG
		descNumberOfDynamicObjectBy(2);
	#endif
									gErrorColumn = gTokens->list[index+4]->column;
									return;
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
						ast = (NMAST*)malloc(sizeof(NMAST));
						ast->valueType = TYPE_FLOATING_POINT;
						ast->sign = 1;
						ast->value = 0;
						ast->parent = NULL;
						ast->variable = tk->text[0];
						if((gTokens->list[index+2]->type == LPAREN ) && (gTokens->list[index+6]->type == RPAREN))
							ast->type = GT_LT;
						else if((gTokens->list[index+2]->type == LPRACKET ) && (gTokens->list[index+6]->type == RPAREN))
							ast->type = GTE_LT;
						else if((gTokens->list[index+2]->type == LPAREN ) && (gTokens->list[index+6]->type == RPRACKET))
							ast->type = GT_LTE;
						else if((gTokens->list[index+2]->type == LPRACKET ) && (gTokens->list[index+6]->type == RPRACKET))
							ast->type = GTE_LTE;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						
						//ast->Left number 1
						ast->left = (NMAST*)malloc(sizeof(NMAST));
						ast->left->valueType = TYPE_FLOATING_POINT;
						ast->left->sign = 1;
						ast->left->left = ast->left->right = NULL;
						ast->left->parent = ast;
						ast->left->value = val;
						ast->left->type = gTokens->list[index+3]->type;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						//Left->Right NUMBER or PI_TYPE or E_TYPE
						ast->right = (NMAST*)malloc(sizeof(NMAST));
						ast->right->valueType = TYPE_FLOATING_POINT;
						ast->right->sign = 1;
						ast->right->left = ast->right->right = NULL;
						ast->right->parent = ast;
						ast->right->value = val2;
						ast->right->type = gTokens->list[index+5]->type;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						pushASTStack(d, ast);
						index += 7;		
					}else{
						/**
							ERROR:After ELEMENT_OF is not a right syntax
						*/
						clearStackWithoutFreeItem(stack, top+1);
						free(stack);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
						for(i=0;i<d->size;i++)
							clearTree(&(d->list[i]));
						free(d->list);
						free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
						gErrorColumn = tk->column;
						gErrorCode = ERROR_SYNTAX;
						return; 
					}
				}else {
					ast = (NMAST*)malloc(sizeof(NMAST));
					ast->valueType = TYPE_FLOATING_POINT;
					ast->sign = 1;
					ast->variable = tk->text[0];
					ast->left = ast->right = ast->parent = NULL;
					ast->value = val;
					ast->type = tk->type;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif					
					
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
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
			for(i=0;i<d->size;i++)
				clearTree(&(d->list[i]));
			free(d->list);
			free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
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
	if(f->domain == NULL){
		f->domain = (NMASTList*)malloc(sizeof(NMASTList));
		f->domain->list = NULL;
		f->domain->loggedSize = 0;
		f->domain->size = 0;
#ifdef DEBUG
		incNumberOfDynamicObject();
#endif
	}
	pushASTStack(f->domain, d->list[0]);
	free(stack);
	free(d);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
}

NMAST* buildIntervalTree(Token* valtk1, Token* o1, Token* variable, Token* o2, Token* valtk2){
	NMAST* ast = NULL;
	NMAST *valNode1, *valNode2;
	DATA_TYPE_FP val1, val2;
	int type, isSwap = FALSE;
	
	/** ERROR cases: -3 < x > 3 or -3 > x < 3  */
	if( ((o1->type == LT || o1->type == LTE) && (o2->type == GT || o2->type == GTE))
				|| ((o1->type == GT || o1->type == GTE) && (o2->type == LT || o2->type == LTE)) ) {
		gErrorCode = ERROR_SYNTAX;
		gErrorColumn = o2->column;
		return NULL;
	}
	
	/** ======================================================================== */
	switch(valtk1->type){
		case NUMBER:
			val1 = parseFloatingPoint(valtk1->text, 0, valtk1->textLength, &gErrorCode);
			if(val1 == 0 && gErrorCode != NMATH_NO_ERROR){
				gErrorColumn = valtk1->column;
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
			val2 = parseFloatingPoint(valtk2->text, 0, valtk2->textLength, &gErrorCode);
			if(val2 == 0 && gErrorCode != NMATH_NO_ERROR){
				gErrorColumn = valtk2->column;
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
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif	
	valNode2 = (NMAST*)malloc(sizeof(NMAST));
	valNode2->variable = 0;
	valNode2->type = valtk2->type;
	valNode2->value = val2;
	valNode2->valueType = TYPE_FLOATING_POINT;
	valNode2->left = valNode2->right = NULL;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
	
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
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif

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