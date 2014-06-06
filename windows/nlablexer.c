#include "nlablexer.h"
#include "common.h"
#include <stdlib.h>

int setLeadNegativeNumber[] = {LPAREN, LPRACKET};
int setNumericOperators[] = {PLUS, MINUS};
Token **tokens = NULL;
int tsize = 0;
int errorColumn = -1;

Token* createToken(int _type, char *_text, int txtlen, int _col){
	int i, l;
	Token *tk = (Token*)malloc(sizeof(Token));
	tk->type = _type;
	tk->column = _col;
	
	l = (MAXTEXTLEN < txtlen)?MAXTEXTLEN:txtlen;
	for(i=0; i<l; i++)
		tk->text[i] = _text[i];
	
	return tk;
}

Token* createTokenIdx(int _type, char *_text, int frIdx, int toIdx, int _col){
	int i, j=0;
	Token *tk = (Token*)malloc(sizeof(Token));
	tk->type = _type;
	tk->column = _col;
	
	//l = (MAXTEXTLEN < txtlen)?MAXTEXTLEN:txtlen;
	for(i=frIdx; i<=toIdx; i++){
		tk->text[j] = _text[i];
		j++;
	}
	
	return tk;
}

int contains(const int *aset, int len, int val){
	int i;
	for(i =0; i<len; i++)
		if(aset[i] == val)
			return TRUE;
	return FALSE;
}
/**********************************************************************/
	
void parseTokens(char *inStr, int length, Token **tokens, int *len){
	int k = 0;
	int idx = 0;
	
	Token *tk = NULL;
		
	while( idx < length ){
		if( isNumericOperatorOREQ(inStr[idx])){
			tk = checkNumericOperator();
			tokens.add(tk);
		}else if( (tk = checkParenthesePrackets(inStr[idx], &idx)) != NULL ){
			tokens.add(tk);
		}else if( (tk = checkCommaSemi(inStr[idx], &idx)) != NULL ){
			tokens.add(tk);
		}else if(isLogicOperator(inStr[idx])){
			k = idx+1;
			k = parserLogicOperator(idx, inStr[idx], k, inStr[k] );
			if( k<0 ) {
				errorColumn = idx;
				return;
			}
			idx = k;
			
		}else if(inStr[idx] == ':' ){
			if(inStr[idx+1] == '-' ){
				tk = new Token(Token.ELEMENT_OF, ":-");
				tk.setColumn(idx);
				tokens.add(tk);
				idx += 2;
			}else{
				errorColumn = idx;
				return;
			}
		}else if(isDigit(inStr[idx])){
			boolean gotFloatingPoint = false;
			for(k = idx+1; k < length; k++){
				if(!isDigit(inStr[k])) {
					if(inStr[k] == '.'){
						//check if we got a floating point
						if(gotFloatingPoint){ //<- the second floating point
							errorColumn = k;
							return;
						}
						gotFloatingPoint = true;
					}else{
						tk = new Token(Token.NUMBER, inStr.substring(idx, k));
						tk.setColumn(idx);
						tokens.add(tk);
						if(inStr[k] == ')'||inStr[k] == ' ' 
								|| isNumericOperatorOREQ(inStr[k]) 
								|| isLogicOperator(inStr[k])) {
							idx = k;
							break;
						}else{
							//Ex: 126a
							//At the moment, I don't handle this case
							//throw Exception
							errorColumn = k;
							return;
						}
					}
				}
			}
			if(idx < k){
				tk = new Token(Token.NUMBER, inStr.substring(idx, k));
				tk.setColumn(idx);
				tokens.add(tk);
				idx = k;
			}
		}else if( (k=isFunctionName(idx))>0 ){
			String fName = inStr.substring(idx, k).toUpperCase();
			tk = new Token(fName, fName);
			tk.setColumn(idx);
			tokens.add(tk);
			idx = k;
		}else if( isVariable(idx)	){
			tk = new Token(Token.NAME, inStr.substring(idx, idx + 1));
			tk.setColumn(idx);
			tokens.add(tk);
			idx++;
		}else if(inStr.charAt(idx)=='o' || inStr.charAt(idx)=='O'){
			if(inStr.charAt(idx+1)=='r' || inStr.charAt(idx+1)=='R'){
				tk = new Token(Token.OR, inStr.substring(idx, idx + 2));
				tk.setColumn(idx);
				tokens.add(tk);
				idx += 2;
			}else{
				//TODO: maybe its a NAME
				idx++;
			}
			
		}else if(inStr.charAt(idx)=='a' || inStr.charAt(idx)=='A'){
			if(inStr.charAt(idx+1)=='n' || inStr.charAt(idx+1)=='N'){
				if(inStr.charAt(idx+2)=='d' || inStr.charAt(idx+2)=='D'){
					tk = new Token(Token.AND, inStr.substring(idx, idx + 3));
					tk.setColumn(idx);
					tokens.add(tk);
					idx += 3;
				}
			}else{
				//TODO: maybe its a NAME
				idx++;
			}
			
		}else
			idx++;
	}
}
	
int parserLogicOperator(int i, char charAtI, int k, char charAtK) {
	Token tk = null;
	int nextPos = -1;
	switch(charAtI){
		case '>':
			if(charAtK == '='){
				tk = new Token(Token.GTE, inputString.substring(i, k+1));
				tk.setColumn(i);
				tokens.add(tk);
				nextPos = k+1;
			}else{
				tk = new Token(Token.GT, inputString.substring(i, k));
				tk.setColumn(i);
				tokens.add(tk);
				nextPos = k;
			}
			break;
		case '<':
			if(charAtK == '='){
				tk = new Token(Token.LTE, inputString.substring(i, k+1));
				tk.setColumn(i);
				tokens.add(tk);
				nextPos = k+1;
			}else{
				tk = new Token(Token.LT, inputString.substring(i, k));
				tk.setColumn(i);
				tokens.add(tk);
				nextPos = k;
			}
			break;
		case '!':
			if(charAtK == '='){
				tk = new Token(Token.NE, inputString.substring(i, k+1));
				tk.setColumn(i);
				tokens.add(tk);
				nextPos = k+1;
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
Token* checkNumericOperator(char *inStr, int length, int *idx){
	Token *tk = NULL;
	if((*idx) >= length)
		return NULL;
		
	char c = inStr[(*idx)];
	switch(c){
		case '+':
			tk = createToken(PLUS, "+", 1, *idx);
			(*idx)++;
		break;
			
		case '-':
			parsSubtractSign(inStr, length, idx);
		break;
			
		case '*':
			tk = createToken(MULTIPLY, "*", 1, *idx);
			tokens.add(tk);
			(*idx)++;
		break;
			
		case '/':
			tk = createToken(DIVIDE, "/", 1, *idx);
			tokens.add(tk);
			(*idx)++;
		break;
			
		case '^':
			tk = createToken(POWER, "^", 1, *idx);
			tokens.add(tk);
			(*idx)++;
		break;
			
		case '=':
			if((*idx)==length-1 || inputString[(*idx)+1] != '>'){
				tk = createToken(EQ, "=", 1, *idx);
				tokens.add(tk);
			} else if(inputString[(*idx)+1] == '>'){
				tk = createToken(IMPLY, "=>", 2, *idx);
				tokens.add(tk);
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
Token* parsSubtractSign(char *inputString, int length, int *idx){
	Token *tk = NULL;
	int k, floatingPoint;
	if((*idx)==length-1){
		//the minus sign is placed at the end of the string, it's a error
		return NULL;
	}
		
	if(inputString[(*idx)+1] != '>'){
		if((((*idx) == 0) || contains(setLeadNegativeNumber, tokens[tsize-1]->type))
				&& (isDigit(inputString[(*idx)+1])) ){
				
			floatingPoint = FALSE;
			for(k = (*idx)+1; k < length; k++){
				if(!isDigit(inputString[k])) {
					if(inputString[k] == '.'){
						//check if we got a floating point
						if(floatingPoint){ //<- the second floating point
							errorColumn = k;
							return;
						}
						floatingPoint = TRUE;
					}else{
						tk = createTokenIdx(NUMBER, inputString, (*idx), k-1, (*idx));
						tokens.add(tk);
						if(inputString[k] == ')'||inputString[k] == ' ' 
								|| isNumericOperatorOREQ(inputString[k]) 
								|| isLogicOperator(inputString[k])) {
							(*idx) = k;
							break;
						}else{
							//Ex: 126a
							//At the moment, I don't handle this case
							//throw Exception
							errorColumn = k;
							return;
						}
					}
				}
			}
			if((*idx) < k){
				tk = createTokenIdx(NUMBER, inputString, (*idx), k-1, (*idx));
				tokens.add(tk);
				(*idx) = k;
			}
			//////////////////////////////////////
		}else{
			tk = createToken(SUBTRACT, "-", 1, (*idx));
			tokens.add(tk);
			(*idx)++;
		}
	}else{ //if(inputString.charAt(index+1)=='>'){
		tk = createToken(RARROW, "->", 2, (*idx));
		tokens.add(tk);
		(*idx) += 2;
	}
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
int isFunctionName(int index, char *inputString, int l){
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
		k = index + 5;
	}else if((index+4 < l) && ((c0=='s' && c1=='q' && c2=='r' && inputString[index+3]=='t' )
			|| (c0=='a' && c1=='t' && c2=='a' && inputString[index+3]=='n') || (c0=='a' && c1=='s' && c2=='i' && inputString[index+3]=='n') ||
			(c0=='a' && c1=='c' && c2=='o' && inputString[index+3]=='s')) ) {
		k = index + 4;
			
	} else if((index+3 < l)&& ((c0=='t' && c1=='a' && c2=='n') || (c0=='s' && c1=='i' && c2=='n') ||
			(c0=='c' && c1=='o' && c2=='s') || (c0=='l' && c1=='o' && c2=='g')))  {
		k = index + 3;
	}else if(c0=='l' && c1=='n'){
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
int isVariable(int index, char *inputString, int length){
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