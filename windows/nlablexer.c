#include <stdlib.h>
#include "nlablexer.h"
#include "common.h"

#define INCLEN 10

#ifndef _WINDOWS
	#define TRUE -1
	#define FALSE 0
#endif

int setLeadNegativeNumber[] = {LPAREN, LPRACKET,SEMI,COMMA,AND,OR,LT,LTE,GT,GTE,EQ,NE,IMPLY,RARROW};
const int LeadNegativeNumberSize = 14;

TokenList *gTokens = NULL;
int gErrorColumn = -1;

int getError(){
	return gErrorColumn;
}

Token* createToken(int _type, const char *_text, int txtlen, int _col){
	int i;
	Token *tk = (Token*)malloc(sizeof(Token));
	tk->type = _type;
	tk->column = _col;
	
	tk->testLength = (MAXTEXTLEN < txtlen)?MAXTEXTLEN:txtlen;
	for(i=0; i<tk->testLength; i++)
		tk->text[i] = _text[i];
	
	
	return tk;
}

Token* createTokenIdx(int _type, const char *_text, int frIdx, int toIdx, int _col){
	int i;
	Token *tk = (Token*)malloc(sizeof(Token));
	tk->type = _type;
	tk->column = _col;
	
	switch(_type){
		case PLUS:
		case MINUS:
		case OR:
			tk->priority = 1;
		break;
		
		case MULTIPLY:
		case DIVIDE:
		case AND:
			tk->priority = 2;
		break;
		
		case POWER:
			tk->priority = 3;
		break;
		
		case NE:
			tk->priority = 4;
		break;
		
		default:
			tk->priority = 0;
	}
	
	//l = (MAXTEXTLEN < txtlen)?MAXTEXTLEN:txtlen;
	tk->testLength = 0;
	for(i=frIdx; i<=toIdx; i++){
		tk->text[tk->testLength] = _text[i];
		(tk->testLength)++;
	}
	return tk;
}

void addToken(TokenList *lst, Token *tk){
	Token **tmp = NULL;
	int i, newLoggedSize;
	if(lst->size >= lst->loggedSize){
		newLoggedSize = lst->loggedSize + INCLEN;
		tmp = (Token**)malloc(sizeof(Token*) * newLoggedSize);
		lst->loggedSize = newLoggedSize;

		for(i = 0; i<lst->size; i++)
			tmp[i] = lst->list[i];

		//release list
		free(lst->list);
		lst->list = tmp;
	}
	lst->list[lst->size] = tk;
	(lst->size)++;
}
/**********************************************************************/
	
void parseTokens(const char *inStr, int length, TokenList *tokens){
	int type, k = 0;
	int idx = 0;
	int floatingPoint = FALSE;
	Token *tk = NULL;
		
	gTokens = tokens;
	
	while( idx < length ){
		if( isNumericOperatorOREQ(inStr[idx])){
			tk = checkNumericOperator(inStr, length, &idx);
			//addToken(tokens, tk);
		}else if( (tk = checkParenthesePrackets(inStr[idx], &idx)) != NULL ){
			addToken(tokens, tk);
		}else if( (tk = checkCommaSemi(inStr[idx], &idx)) != NULL ){
			addToken(tokens, tk);
		}else if(isLogicOperator(inStr[idx])){
			k = idx+1;
			k = parserLogicOperator(inStr, length, idx, inStr[idx], k, inStr[k] );
			if( k<0 ) {
				gErrorColumn = idx;
				return;
			}
			idx = k;
			
		}else if(inStr[idx] == ':' ){
			if(inStr[idx+1] == '-' ){
				tk = createToken(ELEMENT_OF, ":-", 2, idx);
				addToken(tokens, tk);
				idx += 2;
			}else{
				gErrorColumn = idx;
				return;
			}
		}else if(isDigit(inStr[idx])){
			for(k = idx+1; k < length; k++){
				if(!isDigit(inStr[k])) {
					if(inStr[k] == '.'){
						//check if we got a floating point
						if(floatingPoint){ //<- the second floating point
							gErrorColumn = k;
							return;
						}
						floatingPoint = TRUE;
					}else{
						tk = createTokenIdx(NUMBER, inStr, idx, k-1, idx);
						addToken(tokens, tk);
						if(inStr[k] == ')'||inStr[k] == ' ' 
								|| isNumericOperatorOREQ(inStr[k]) 
								|| isLogicOperator(inStr[k])) {
							idx = k;
							break;
						}else{
							//Ex: 126a
							//At the moment, I don't handle this case
							//throw Exception
							gErrorColumn = k;
							return;
						}
					}
				}
			}
			if(idx < k){
				tk = createTokenIdx(NUMBER, inStr, idx, k-1, idx);
				addToken(tokens, tk);
				idx = k;
			}
		}else if( (k=isFunctionName(idx, inStr, length, &type ))>0 ){
			tk = createTokenIdx(type, inStr, idx, k-1, idx);
			addToken(tokens, tk);
			idx = k;
		}else if( isVariable(idx, inStr, length) ){
			//tk = createTokenIdx(NAME, inStr, idx, idx, idx);
			tk = createTokenIdx(VARIABLE, inStr, idx, idx, idx);
			addToken(tokens, tk);
			idx++;
		}else if(inStr[idx]=='o' || inStr[idx]=='O'){
			if(inStr[idx+1]=='r' || inStr[idx+1]=='R'){
				tk = createTokenIdx(OR, inStr, idx, idx+1, idx);
				addToken(tokens, tk);
				idx += 2;
			}else{
				//TODO: maybe its a NAME
				idx++;
			}
			
		}else if(inStr[idx]=='a' || inStr[idx]=='A'){
			if(inStr[idx+1]=='n' || inStr[idx+1]=='N'){
				if(inStr[idx+2]=='d' || inStr[idx+2]=='D'){
					tk = createTokenIdx(AND, inStr, idx, idx+2, idx);
					addToken(tokens, tk);
					idx += 3;
				}
			}else{
				//TODO: maybe its a NAME
				idx++;
			}
			
		}else
			idx++;
	}
	gTokens = NULL;
}
	
int parserLogicOperator(const char *inStr, int length, int i, char charAtI, int k, char charAtK) {
	Token *tk = NULL;
	int nextPos = -1;

	switch(charAtI){
		case '>':
			if(charAtK == '='){
				tk = createTokenIdx(GTE, inStr, i, k, i);
				addToken(gTokens, tk);
				nextPos = k+1;
			}else{
				tk = createTokenIdx(GT, inStr, i, i, i);
				addToken(gTokens, tk);
				nextPos = i+1;
			}
			break;
		case '<':
			if(charAtK == '='){
				tk = createTokenIdx(LTE, inStr, i, k, i);
				addToken(gTokens, tk);
				nextPos = k+1;
			}else{
				tk = createTokenIdx(LT, inStr, i, i, i);
				addToken(gTokens, tk);
				nextPos = i+1;
			}
			break;
		case '!':
			if(charAtK == '='){
				tk = createTokenIdx(NE, inStr, i, i+1, i);
				addToken(gTokens, tk);
				nextPos = i+2;
			}
			break;
	}
	return nextPos;
}

Token* checkParenthesePrackets(char c, int *idx){
	Token *tk = NULL;
	switch(c){
		case '(':
			tk = createToken(LPAREN, "(", 1, *idx);
			(*idx)++;
		break;
		
		case ')':
			tk = createToken(RPAREN, ")", 1, *idx);
			(*idx)++;
		break;
			
		case '[':
			tk = createToken(LPRACKET, "[", 1, *idx);
			(*idx)++;
		break;
			
		case ']':
			tk = createToken(RPRACKET, "]", 1, *idx);
			(*idx)++;
		break;
	}
	return tk;
}
	
Token* checkCommaSemi(char c, int *idx){
	Token *tk = NULL;
	switch(c){
		case ',':
			tk = createToken(COMMA, ",", 1, *idx);
			(*idx)++;
		break;
		
		case ';':
			tk = createToken(SEMI, ";", 1, *idx);
			(*idx)++;
		break;
	}
		
	return tk;
}
	
/***********************************************************************/
/**
 * @see isNumericOperatorOREQ
 * @return errorCode
 */
Token* checkNumericOperator(const char *inStr, int length, int *idx){
	Token *tk = NULL;
	char c;

	if((*idx) >= length)
		return NULL;

	c = inStr[(*idx)];
	switch(c){
		case '+':
			tk = createToken(PLUS, "+", 1, *idx);
			addToken(gTokens, tk);
			(*idx)++;
		break;
			
		case '-':
			parsSubtractSign(inStr, length, idx);
		break;
			
		case '*':
			tk = createToken(MULTIPLY, "*", 1, *idx);
			addToken(gTokens, tk);
			(*idx)++;
		break;
			
		case '/':
			tk = createToken(DIVIDE, "/", 1, *idx);
			addToken(gTokens, tk);
			(*idx)++;
		break;
			
		case '^':
			tk = createToken(POWER, "^", 1, *idx);
			addToken(gTokens, tk);
			(*idx)++;
		break;
			
		case '=':
			if((*idx)==length-1 || inStr[(*idx)+1] != '>'){
				tk = createToken(EQ, "=", 1, *idx);
				addToken(gTokens, tk);
			} else if(inStr[(*idx)+1] == '>'){
				tk = createToken(IMPLY, "=>", 2, *idx);
				addToken(gTokens, tk);
				(*idx)++;
			}
			(*idx)++;
		break;
	}
		
	return tk;
}
	
int isNumericOperatorOREQ(char c){
	switch(c){
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
		case '=':
			return TRUE;
	}
		
	return FALSE;
}
	/***********************************************************************/
	
/**
 * Just call this routine if character at currentPos is a minus sign
 * Example
 */
Token* parsSubtractSign(const char *inputString, int length, int *idx){
	Token *tk = NULL;
	int k, floatingPoint;
	if((*idx)==length-1){
		//the minus sign is placed at the end of the string, it's a error
		return NULL;
	}
		
	if(inputString[(*idx)+1] != '>'){
		if ((((*idx) == 0) || contains(gTokens->list[gTokens->size-1]->type, setLeadNegativeNumber, LeadNegativeNumberSize))
				&& (isDigit(inputString[(*idx)+1]))){
				
			floatingPoint = FALSE;
			for(k = (*idx)+1; k < length; k++){
				if(!isDigit(inputString[k])) {
					if(inputString[k] == '.'){
						//check if we got a floating point
						if(floatingPoint){ //<- the second floating point
							gErrorColumn = k;
							return NULL;
						}
						floatingPoint = TRUE;
					}else{
						tk = createTokenIdx(NUMBER, inputString, (*idx), k-1, (*idx));
						addToken(gTokens, tk);
						if(inputString[k] == ')'||inputString[k] == ' ' 
								|| isNumericOperatorOREQ(inputString[k]) 
								|| isLogicOperator(inputString[k])) {
							(*idx) = k;
							break;
						}else{
							//Ex: 126a
							//At the moment, I don't handle this case
							//throw Exception
							gErrorColumn = k;
							return NULL;
						}
					}
				}
			}
			if((*idx) < k){
				tk = createTokenIdx(NUMBER, inputString, (*idx), k-1, (*idx));
				addToken(gTokens, tk);
				(*idx) = k;
			}
			//////////////////////////////////////
		}else{
			tk = createToken(MINUS, "-", 1, (*idx));
			addToken(gTokens, tk);
			(*idx)++;
		}
	}else{ //if(inputString.charAt(index+1)=='>'){
		tk = createToken(RARROW, "->", 2, (*idx));
		addToken(gTokens, tk);
		(*idx) += 2;
	}
	return tk;
}
	
int isLogicOperator(char c){
	switch(c){
		case '>':
		case '<':
		case '!':
			return TRUE;
	}
	return FALSE;
}
	
int isDigit(char c){
	if(c>=48 && c<=57)
		return TRUE;
	return FALSE;
}
	
/**
	return the position where a function name is end
	return -1 if it do not match any function name
*/
int isFunctionName(int index, const char *inputString, int l, int *outType){
	int k = -1;	
	char c0, c1, c2;
		
	if(index+2 < l){
		c0 = inputString[index];
		c1 = inputString[index+1];
		c2 = inputString[index+2];
	}else
		return -1;
			
	if( (index+5 < l) && c0=='c' && c1=='o' && c2 =='t' && 
				(inputString[index+3]=='a')	&& (inputString[index+4]=='n') ){
		(*outType) = COTAN;
		k = index + 5;
	}else if((index+4 < l) && (c0=='s' && c1=='q' && c2=='r' && inputString[index+3]=='t' )){
		(*outType) = SQRT;
		k = index + 4;
	}else if((index+4 < l) && (c0=='a' && c1=='t' && c2=='a' && inputString[index+3]=='n')){
		(*outType) = ATAN;
		k = index + 4;
	}else if((index+4 < l) && (c0=='a' && c1=='s' && c2=='i' && inputString[index+3]=='n')){
		(*outType) = ASIN;
		k = index + 4;
	}else if((index+4 < l) && (c0=='a' && c1=='c' && c2=='o' && inputString[index+3]=='s')) {
		(*outType) = ACOS;
		k = index + 4;
	} else if((index+3 < l)&& (c0=='t' && c1=='a' && c2=='n')){
		(*outType) = TAN;
		k = index + 3;
	}else if((index+3 < l) && (c0=='s' && c1=='i' && c2=='n')){
		(*outType) = SIN;
		k = index + 3;
	}else if((index+3 < l) &&(c0=='c' && c1=='o' && c2=='s')){
		(*outType) = COS;
		k = index + 3;
	}else if((index+3 < l) && (c0=='l' && c1=='o' && c2=='g')) {
		(*outType) = LOG;
		k = index + 3;
	}else if(c0=='l' && c1=='n'){
		(*outType) = LN;
		k = index + 2;
	}
		
	if(k>=0 && inputString[k]!='('){
			return -1;
	}
	return k;
}
	
/**
	A variable is always a single character NAME.
	If a single character followed by (){}[];+-* / , ; > < ! space then it's a VARIABLE
*/
int isVariable(int index, const char *inputString, int length){
	char cc = inputString[index];
	char nextC;
		
	if(( cc>= 'a' && cc<='z' ) || (cc>= 'A' && cc<='Z')){
		if((index < length-1)){
			nextC = inputString[index + 1];
			if(nextC==' ' || nextC=='+' || nextC=='-' || nextC=='*' || nextC=='/' || nextC=='^' || nextC=='='
					|| nextC=='(' || nextC==')' || nextC=='[' || nextC==']' || nextC=='<' || nextC=='>'
					|| nextC=='!'|| nextC==','|| nextC==';')
				return TRUE;
		}else
			return TRUE;
	}
	return FALSE;
}