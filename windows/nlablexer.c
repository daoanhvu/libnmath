#include "nlablexer.h"

int currentIdx = 0;

int setLeadNegativeNumber[] = {};
int setNumericOperators[] = {};
Token *tokens;
	
	
void parseTokens(char *inputString, int length){
	int k = 0;
	Token tk = null;
	currentIdx = 0;
		
	while( currentIdx < length ){
		if( isNumericOperatorOREQ(inputString[currentIdx])){
			checkNumericOperator();
		}else if( (tk = checkParenthesePrackets(inputString[currentIdx])) != null ){
			tokens.add(tk);
			tk.setColumn(currentIdx);
			currentIdx++;
		}else if( (tk = checkCommaSemi(inputString[currentIdx]))!=null ){
			tokens.add(tk);
			tk.setColumn(currentIdx);
			currentIdx++;
		}else if(isLogicOperator(inputString[currentIdx])){
			k = currentIdx+1;
			k = parserLogicOperator(currentIdx, inputString[currentIdx], k, inputString[k] );
			if( k<0 ) {
				errorColumn = currentIdx;
				return;
			}
			currentIdx = k;
			
		}else if(inputString.charAt(currentIdx) == ':' ){
			if(inputString.charAt(currentIdx+1) == '-' ){
				tk = new Token(Token.ELEMENT_OF, ":-");
				tk.setColumn(currentIdx);
				tokens.add(tk);
				currentIdx += 2;
			}else{
				errorColumn = currentIdx;
				return;
			}
		}else if(isDigit(inputString.charAt(currentIdx))){
			boolean gotFloatingPoint = false;
			for(k = currentIdx+1; k < inputString.length(); k++){
				if(!isDigit(inputString.charAt(k))) {
					if(inputString.charAt(k) == '.'){
						//check if we got a floating point
						if(gotFloatingPoint){ //<- the second floating point
							errorColumn = k;
							return;
						}
						gotFloatingPoint = true;
					}else{
						tk = new Token(Token.NUMBER, inputString.substring(currentIdx, k));
						tk.setColumn(currentIdx);
						tokens.add(tk);
						if(inputString.charAt(k) == ')'||inputString.charAt(k) == ' ' 
								|| isNumericOperatorOREQ(inputString.charAt(k)) 
								|| isLogicOperator(inputString.charAt(k))) {
							currentIdx = k;
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
			if(currentIdx < k){
				tk = new Token(Token.NUMBER, inputString.substring(currentIdx, k));
				tk.setColumn(currentIdx);
				tokens.add(tk);
				currentIdx = k;
			}
		}else if( (k=isFunctionName(currentIdx))>0 ){
			String fName = inputString.substring(currentIdx, k).toUpperCase();
			tk = new Token(fName, fName);
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx = k;
		}else if( isVariable(currentIdx)	){
			tk = new Token(Token.NAME, inputString.substring(currentIdx, currentIdx + 1));
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx++;
		}else if(inputString.charAt(currentIdx)=='o' || inputString.charAt(currentIdx)=='O'){
			if(inputString.charAt(currentIdx+1)=='r' || inputString.charAt(currentIdx+1)=='R'){
				tk = new Token(Token.OR, inputString.substring(currentIdx, currentIdx + 2));
				tk.setColumn(currentIdx);
				tokens.add(tk);
				currentIdx += 2;
			}else{
				//TODO: maybe its a NAME
				currentIdx++;
			}
			
		}else if(inputString.charAt(currentIdx)=='a' || inputString.charAt(currentIdx)=='A'){
			if(inputString.charAt(currentIdx+1)=='n' || inputString.charAt(currentIdx+1)=='N'){
				if(inputString.charAt(currentIdx+2)=='d' || inputString.charAt(currentIdx+2)=='D'){
					tk = new Token(Token.AND, inputString.substring(currentIdx, currentIdx + 3));
					tk.setColumn(currentIdx);
					tokens.add(tk);
					currentIdx += 3;
				}
			}else{
				//TODO: maybe its a NAME
				currentIdx++;
			}
			
		}else
			currentIdx++;
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

void checkParenthesePrackets(char c, Token * tk){
	switch(c){
		case '(':
			tk = new Token(Token.LPAREN, "(");
		break;
		
		case ')':
			tk = new Token(Token.RPAREN, ")");				
		break;
			
		case '[':
			tk = new Token(Token.LPRACKET, "[");				
		break;
			
		case ']':
			tk = new Token(Token.RPRACKET, "]");				
		break;
	}
}
	
private Token checkCommaSemi(char c){
	Token tk = null;
	switch(c){
		case ',':
			tk = new Token(Token.COMMA, ",");
		break;
		
		case ';':
			tk = new Token(Token.SEMI, ";");				
		break;
	}
		
	return tk;
}
	
/***********************************************************************/
/**
 * @see isNumericOperatorOREQ
 * @return errorCode
 */
int checkNumericOperator(Token *tk){
	
	if(currentIdx >= inputString.length())
		return -1;
		
	char c = inputString.charAt(currentIdx);
	switch(c){
		case '+':
			tk = new Token(Token.PLUS, "+");
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx++;
		break;
			
		case '-':
			parsSubtractSign();
		break;
			
		case '*':
			tk = new Token(Token.MULTIPLY, "*");
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx++;
		break;
			
		case '/':
			tk = new Token(Token.DIVIDE, "/");
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx++;
		break;
			
		case '^':
			tk = new Token(Token.POWER, "^");
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx++;
		break;
			
		case '=':
			if(currentIdx==inputString.length()-1 || inputString.charAt(currentIdx+1) != '>'){
				tk = new Token(Token.EQ, "=");
				tk.setColumn(currentIdx);
				tokens.add(tk);
			} else if(inputString.charAt(currentIdx+1) == '>'){
				tk = new Token(Token.IMPLY, "=>");
				tk.setColumn(currentIdx);
				tokens.add(tk);
				currentIdx++;
			}
			currentIdx++;
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
void parsSubtractSign(char *inputString, int length){
	Token tk = null;
	int k, gotFloatingPoint;
	if(currentIdx==length-1){
		//the minus sign is placed at the end of the string, it's a error
		return;
	}
		
	if(inputString[currentIdx+1] != '>'){
			if(((currentIdx == 0) || NLabLexer.setLeadNegativeNumber.contains((tokens.get(tokens.size()-1))))
					&& (isDigit(inputString[currentIdx+1])) ){
				
				gotFloatingPoint = FALSE;
				for(k = currentIdx+1; k < inputString.length(); k++){
					if(!isDigit(inputString[k])) {
						if(inputString.charAt(k) == '.'){
							//check if we got a floating point
							if(gotFloatingPoint){ //<- the second floating point
								errorColumn = k;
								return;
							}
							gotFloatingPoint = TRUE;
						}else{
							tk = new Token(Token.NUMBER, inputString.substring(currentIdx, k));
							tk.setColumn(currentIdx);
							tokens.add(tk);
							if(inputString[k] == ')'||inputString[k] == ' ' 
									|| isNumericOperatorOREQ(inputString[k]) 
									|| isLogicOperator(inputString[k])) {
								currentIdx = k;
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
				
				if(currentIdx < k){
					tk = new Token(Token.NUMBER, inputString.substring(currentIdx, k));
					tk.setColumn(currentIdx);
					tokens.add(tk);
					currentIdx = k;
				}
				//////////////////////////////////////
			}else{
				tk = new Token(Token.SUBTRACT, "-");
				tk.setColumn(currentIdx);
				tokens.add(tk);
				currentIdx++;
			}
		}else{ //if(inputString.charAt(index+1)=='>'){
			tk = new Token(Token.RARROW, "->");
			tk.setColumn(currentIdx);
			tokens.add(tk);
			currentIdx += 2;
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