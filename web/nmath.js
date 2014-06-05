/********************************************************************************************/
var LPAREN = 0;
var LPRACKET = 1;
var SEMI = 2;
var COMMA = 3;
var AND = 4;
var OR = 5;
var LT = 6;
var LTE = 7;
var GT = 8;
var GTE = 9;
var EQ = 10;
var NE = 11;
var IMPLY = 12;
var RARROW = 13;
var PLUS = 14;
var SUBTRACT = 15;
var MULTIPLY = 16;
var DIVIDE = 17;
var POWER = 18;
									

/********************************************************************************************/

var currentIdx = -1;
var inputString = "";
var tokens = new Array();

/**
		 * negative number -2
		 * 					(-2
		 * 					[-2
		 * 					;-2
		 * 					,-2
		 * 					AND -2
		 * 					OR -2
		 * 					< -2
		 * 					<= -2
		 * 					> -2
		 * 					>= -2
		 * 					= -2
		 * 					!= -1
		 * 					=> -2
		 * 					-> -2
		 */
var setLeadNegativeNumber = [LPAREN,
									LPRACKET,
									SEMI,
									COMMA,
									AND,
									OR,
									LT,
									LTE,
									GT,
									GTE,
									EQ,
									NE,
									IMPLY,
									RARROW];

/**
 * + - * / ^
 */									
var setNumericOperators = [PLUS,
									SUBTRACT,
									MULTIPLY,
									DIVIDE,
									POWER];
									
/********************************************************************************************/

function createToken(_type, _text, _col){
	var obj = { type:_type, text:_text, column:_col	};
	return obj;
}
	
function parseTokens(String inputStr){
	var k = 0;
	var tk = null;
	var gotFloatingPoint = false;
	
	currentIdx = 0;
	inputString = inputStr;
		
	while( currentIdx < inputString.length() ){
		if( isNumericOperatorOREQ(inputString.charAt(currentIdx))){
			checkNumericOperator();
		}else if( (tk = checkParenthesePrackets(inputString.charAt(currentIdx))) != null ){
			tokens.push(tk);
			tk.setColumn(currentIdx);
			currentIdx++;
		}else if( (tk = checkCommaSemi(inputString.charAt(currentIdx)))!=null ){
			tokens.push(tk);
			tk.setColumn(currentIdx);
			currentIdx++;
		}else if(isLogicOperator(inputString.charAt(currentIdx))){
			k = currentIdx+1;
			k = parserLogicOperator(currentIdx, inputString.charAt(currentIdx), k, inputString.charAt(k) );
			if(k<0){
				errorColumn = currentIdx;
				return;
			}
			currentIdx = k;
				
		}else if(inputString.charAt(currentIdx) == ':' ){
			if(inputString.charAt(currentIdx+1) == '-' ){
				tk = createToken(ELEMENT_OF, ':-', currentIdx);
				tokens.push(tk);
				currentIdx += 2;
			}else{
				errorColumn = currentIdx;
				return;
			}
		}else if(isDigit(inputString.charAt(currentIdx))){
			gotFloatingPoint = false;
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
						tokens.push(tk);
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
					tokens.push(tk);
					currentIdx = k;
				}
		}else if( (k=isFunctionName(currentIdx))>0 ){
			String fName = inputString.substring(currentIdx, k).toUpperCase();
			tk = new Token(fName, fName);
			tk.setColumn(currentIdx);
			tokens.push(tk);
			currentIdx = k;
		}else if( isVariable(currentIdx)	){
			tk = new Token(Token.NAME, inputString.substring(currentIdx, currentIdx + 1));
			tk.setColumn(currentIdx);
			tokens.push(tk);
			currentIdx++;
		}else if(inputString.charAt(currentIdx)=='o' || inputString.charAt(currentIdx)=='O'){
			if(inputString.charAt(currentIdx+1)=='r' || inputString.charAt(currentIdx+1)=='R'){
				tk = new Token(Token.OR, inputString.substring(currentIdx, currentIdx + 2));
				tk.setColumn(currentIdx);
				tokens.push(tk);
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
					tokens.push(tk);
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

/*
	Check if position from i form a Logic operator
	return the next position
*/	
function parserLogicOperator(i, charAtI, k, charAtK) {
	Token tk = null;
	var nextPos = -1;
	switch(charAtI){
		case '>':
			if(charAtK == '='){
				tk = new Token(Token.GTE, inputString.substring(i, k+1));
				tk.setColumn(i);
				tokens.push(tk);
				nextPos = k+1;
			}else{
				tk = new Token(Token.GT, inputString.substring(i, k));
				tk.setColumn(i);
				tokens.push(tk);
				nextPos = k;
			}
			break;
		case '<':
			if(charAtK == '='){
				tk = new Token(Token.LTE, inputString.substring(i, k+1));
				tk.setColumn(i);
				tokens.push(tk);
				nextPos = k+1;
			}else{
				tk = new Token(Token.LT, inputString.substring(i, k));
				tk.setColumn(i);
				tokens.push(tk);
				nextPos = k;
			}
			break;
		case '!':
			if(charAtK == '='){
				tk = new Token(Token.NE, inputString.substring(i, k+1));
				tk.setColumn(i);
				tokens.push(tk);
				nextPos = k+1;
			}
			break;
	}
	return nextPos;
}

Token checkParenthesePrackets(char c){
		Token tk = null;
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
		return tk;
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
	 * @return
	 */
	private Token checkNumericOperator(){
		Token tk = null;
		
		if(currentIdx >= inputString.length())
			return null;
		char c = inputString.charAt(currentIdx);
		switch(c){
			case '+':
				tk = new Token(Token.PLUS, "+");
				tk.setColumn(currentIdx);
				tokens.push(tk);
				currentIdx++;
			break;
			
			case '-':
				parsSubtractSign();
			break;
			
			case '*':
				tk = new Token(Token.MULTIPLY, "*");
				tk.setColumn(currentIdx);
				tokens.push(tk);
				currentIdx++;
			break;
			
			case '/':
				tk = new Token(Token.DIVIDE, "/");
				tk.setColumn(currentIdx);
				tokens.push(tk);
				currentIdx++;
			break;
			
			case '^':
				tk = new Token(Token.POWER, "^");
				tk.setColumn(currentIdx);
				tokens.push(tk);
				currentIdx++;
			break;
			
			case '=':
				if(currentIdx==inputString.length()-1 || inputString.charAt(currentIdx+1) != '>'){
					tk = new Token(Token.EQ, "=");
					tk.setColumn(currentIdx);
					tokens.push(tk);
				} else if(inputString.charAt(currentIdx+1) == '>'){
					tk = new Token(Token.IMPLY, "=>");
					tk.setColumn(currentIdx);
					tokens.push(tk);
					currentIdx++;
				}
				currentIdx++;
			break;
		}
		
		return tk;
	}
	
	private boolean isNumericOperatorOREQ(char c){
		switch(c){
			case '+':
			case '-':
			case '*':
			case '/':
			case '^':
			case '=':
				return true;
		}
		
		return false;
	}
	/***********************************************************************/

/**
 * Just call this routine if character at currentPos is a minus sign
 * Example
 */
function parsSubtractSign(){
	var tk = null;
	var k;
	
	if(currentIdx==inputString.length-1){
		//the minus sign is placed at the end of the string, it's a error
		return;
	}
		
	if(inputString.charAt(currentIdx+1) != '>'){
		if(((currentIdx == 0) || NLabLexer.setLeadNegativeNumber.contains((tokens.get(tokens.size()-1))))
				&& (isDigit(inputString.charAt(currentIdx+1))) ){
							
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
						tokens.push(tk);
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
				tokens.push(tk);
				currentIdx = k;
			}
			//////////////////////////////////////
		}else{
			tk = new Token(Token.SUBTRACT, "-");
			tk.setColumn(currentIdx);
			tokens.push(tk);
			currentIdx++;
		}
	}else{ //if(inputString.charAt(index+1)=='>'){
		tk = new Token(Token.RARROW, "->");
		tk.setColumn(currentIdx);
		tokens.push(tk);
		currentIdx += 2;
	}
}

//
//
//
//

function isLogicOperator(c){
	switch(c){
		case '>':
		case '<':
		case '!':
			return true;
	}
	return false;
}
	
function isDigit(c){
	var code = c.charCodeAt();
	if(code>=48 && code<=57)
		return true;
	return false;
}
/**
	return the position where a function name is end
	return -1 if it do not match any function name
*/
function isFunctionName(idx, inputString){
	var k = -1;
	var l = inputString.length;
	var c0, c1, c2;
	
	if(idx+2 < inputString.length){
		c0 = inputString.charAt(idx);
		c1 = inputString.charAt(idx+1);
		c2 = inputString.charAt(idx+2);
	}else
		return -1;
	
	if( (idx+5 < l) && c0=='c' && c1=='o' && c2 =='t' && 
				(inputString.charAt(idx+3)=='a')	&& (inputString.charAt(idx+4)=='n') ){
		k = idx + 5;
	}else if((idx+4 < l) && ((c0=='s' && c1=='q' && c2=='r' && inputString.charAt(idx+3)=='t' )
			|| (c0=='a' && c1=='t' && c2=='a' && inputString.charAt(idx+3)=='n') || (c0=='a' && c1=='s' && c2=='i' && inputString.charAt(idx+3)=='n') ||
			(c0=='a' && c1=='c' && c2=='o' && inputString.charAt(idx+3)=='s')) ) {
		k = idx + 4;
		
	} else if((idx+3 < l)&& ((c0=='t' && c1=='a' && c2=='n') || (c0=='s' && c1=='i' && c2=='n') ||
			(c0=='c' && c1=='o' && c2=='s') || (c0=='l' && c1=='o' && c2=='g')))  {
		k = idx + 3;
	}else if(c0=='l' && c1=='n'){
		k = idx + 2;
	}
		
	if(k>=0 && inputString.charAt(k)!='('){
		return -1;
	}
	return k;
}

/**
	A variable is always a single character NAME.
	If a single character followed by (){}[];+-* / , ; > < ! space then it's a VARIABLE
*/
function isVariable(idx, inputString){
	var cc = inputString.charCodeAt(idx);
	var nextC;
	var length = inputString.length;
	
	if(( cc>= 'a'.charCodeAt() && cc<='z'.charCodeAt() ) || (cc>= 'A'.charCodeAt() && cc<='Z'.charCodeAt())){
		if((idx < length-1)){
			nextC = inputString.charAt(idx + 1);
			if(nextC==' ' || nextC=='+' || nextC=='-' || nextC=='*' || nextC=='/' || nextC=='^' || nextC=='='
					|| nextC=='(' || nextC==')' || nextC=='[' || nextC==']' || nextC=='<' || nextC=='>'
					|| nextC=='!'|| nextC==','|| nextC==';')
				return true;
		}else
			return true;
	}
	return false;
}