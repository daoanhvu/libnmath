#include <stdlib.h>
#include <string.h>
#include "nlablexer.h"

#ifdef _TARGET_HOST_ANDROID
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NLABLEXER"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

const int setLeadNegativeNumber[] = {LPAREN, LPRACKET,SEMI,COMMA,AND,OR,LT,LTE,GT,GTE,EQ,NE,IMPLY,RARROW};
const int LeadNegativeNumberSize = 14;

extern int gErrorColumn;
extern int gErrorCode;

/**
	Add a token info into list, size of the list will be increased one

	if SUCCESS, return the size of token
	otherwise return a negative value
*/
int addToken(TokenList *lst, int _type, const char *_text, char txtlen, int _col) {

	if(lst->size >= lst->loggedSize) {
		return E_NOT_ENOUGH_PLACE;
	}

	lst->list[lst->size].type = _type;
	lst->list[lst->size].column = _col;

	/*
		IMPORTANT: If you want to change this, PLEASE change function common:getPriorityOfType(int type) also
	*/
	switch(_type){
		case OR:
			lst->list[lst->size].priority = 1;
		break;
		
		case AND:
			lst->list[lst->size].priority = 2;
		break;
		
		case LT:
		case GT:
		case LTE:
		case GTE:
			lst->list[lst->size].priority = 3;
		break;
		
		case PLUS:
		case MINUS:
			lst->list[lst->size].priority = 4;
		break;
		
		case MULTIPLY:
		case DIVIDE:
			lst->list[lst->size].priority = 5;
		break;
		
		case POWER:
			lst->list[lst->size].priority = 6;
		break;
		
		case NE:
			lst->list[lst->size].priority = 7;
		break;
		
		default:
			lst->list[lst->size].priority = 0;
	}
	
	lst->list[lst->size].textLength = (char)((MAXTEXTLEN < txtlen)?MAXTEXTLEN:txtlen);
	memcpy(lst->list[lst->size].text, _text, lst->list[lst->size].textLength);

	(lst->size)++;

	return lst->size;
}

/**********************************************************************/

/*
	return the UTF-8 character code at index of the string
*/
int getCharacter(const char *str, int length, int index, int *nextIdx) {

	int result = str[index] & 0x000000FF;

	/*
	if((inStr[idx] & 0x80) != 0x80) {
		*nextIdx = index + 1;
		return result;
	}
	*/

	gErrorCode = NMATH_NO_ERROR;
	if( (str[index] & 0xF8) == 0xF0) {
		/* 11110XXX We need to read three bytes more */
		result = str[index] & 0x00000007;
		result = ((( result  << 18) | ((str[index+1] & 0x0000003F) << 12)) | ((str[index+2] & 0x0000003F) << 6)) | (str[index+3] & 0x0000003F);
		*nextIdx = index + 4;
	} else if( (str[index] & 0xF0) == 0xE0) {
		/* 1110XXXX We need to read two bytes more */
		result = str[index] & 0x0000000F;
		result = (( result  << 12) | ((str[index+1] & 0x0000003F) << 6)) | (str[index+2] & 0x0000003F);
		*nextIdx = index + 3;
	} else if ( (str[index] & 0xE0) == 0xC0) {
		/* 110XXXXX We need to read one byte more */
		result = str[index] & 0x0000001F;
		result = (result << 6) | (str[index+1] & 0x0000003f);
		*nextIdx = index + 2;
	} else {
		gErrorCode = ERROR_MALFORMED_ENCODING;
	}

	return result;
}

void lexicalAnalysisUTF8(const char *inStr, int length, TokenList *tokens, int appended) {
	int chCode, type, k = 0;
	int idx = 0, nextIdx;
	int floatingPoint;

	gErrorColumn = -1;
	gErrorCode = NMATH_NO_ERROR;

	if( (appended==TRUE) && (tokens->size > 0) ) {
		idx = tokens->list[tokens->size-1].column;
		tokens->size -= 1;
	}

	while( idx < length ) {
		//LOGI(3, "index: %d, 0x%X(%d)", idx, inStr[idx], (char)(inStr[idx]));
		if( (inStr[idx] & 0x80) != 0x80 ) {
			if( checkNumericOperator(tokens, inStr, length, idx, &type, &k )==TRUE ) {
				addToken(tokens, type, inStr + idx, k, idx);
				idx += k;
			}else if( checkParenthesePrackets(inStr[idx], &type) == TRUE ) {
				addToken(tokens, type, inStr + idx, 1, idx);
				idx++;
			}else if( checkCommaSemi(inStr[idx], &type) == TRUE ) {
				addToken(tokens, type, inStr + idx, 1, idx);
				idx++;
			}else if(parserLogicOperator(inStr, length, idx, &type, &k ) == TRUE) {
				addToken(tokens, type, inStr + idx, k, idx);
				idx += k;
			}else if(inStr[idx] == ':' ) {
				if(inStr[idx+1] == '-' ) {
					addToken(tokens, ELEMENT_OF, inStr + idx, 2, idx);
					idx += 2;
				}else{ //ERROR: bad token found
					gErrorColumn = idx;
					gErrorCode = ERROR_BAD_TOKEN;
					return;
				}
			}else if(isDigit(inStr[idx])) {
				floatingPoint = FALSE;
				for(k = idx+1; k < length; k++) {
					if(!isDigit(inStr[k])) {
						if(inStr[k] == '.') {
							//check if we got a floating point
							if(floatingPoint){ //<- ERROR: the second floating point
								gErrorColumn = k;
								gErrorCode = ERROR_TOO_MANY_FLOATING_POINT;
								return;
							}
							floatingPoint = TRUE;
						} else {
							addToken(tokens, NUMBER, inStr + idx, k-idx, idx);
							idx = k;
							break;
						}
					}
				}
				if(idx < k){
					addToken(tokens, NUMBER, inStr + idx, k-idx, idx);
					idx = k;
				}
			}else if( isFunctionName(inStr, length, idx, &type, &k ) == TRUE ) {
				addToken(tokens, type, inStr + idx, k, idx);
				idx += k;
			}else if(idx>0 && (inStr[idx-1]==' ') && (inStr[idx]=='D') && (inStr[idx+1]==':') ){
				addToken(tokens, DOMAIN_NOTATION, "DOMAIN_NOTATION", 14, idx);
				idx += 2;
			}else if( isAName(inStr, length, idx) ) {
				addToken(tokens, NAME, inStr + idx, 1, idx);
				idx++;
			}else if(inStr[idx]=='o' || inStr[idx]=='O') {
				if(inStr[idx+1]=='r' || inStr[idx+1]=='R'){
					addToken(tokens, OR, inStr + idx, 2, idx);
					idx += 2;
				}else{
					//TODO: maybe its a NAME
					idx++;
				}
				
			}else if(inStr[idx]=='a' || inStr[idx]=='A'){
				if(inStr[idx+1]=='n' || inStr[idx+1]=='N'){
					if(inStr[idx+2]=='d' || inStr[idx+2]=='D'){
						addToken(tokens, AND, inStr + idx, 3, idx);
						idx += 3;
					}
				}else{
					//TODO: maybe its a NAME
					idx++;
				}
				
			}else if( (idx+1 < length ) && (inStr[idx]=='p' || inStr[idx]=='P') && (inStr[idx+1]=='i' || inStr[idx+1]=='I')
							&& ( (idx+1 == length-1) || !isLetter(inStr[idx+2]) ) ) {
				addToken(tokens, PI_TYPE, "3.14159265358979", 16, idx);
				idx += 2;
			}else if(inStr[idx]=='e' && ((idx==length-1) || !isLetter(inStr[idx+1]))) {
				addToken(tokens, E_TYPE, "2.718281828", 11, idx);
				idx++;
			}else
				idx++;

		} else {

			chCode = getCharacter(inStr, length, idx, &nextIdx);
			//LOGI(3, "UTF Code: (0x%X)%d", chCode, chCode);
			switch(chCode) {
				case PI_TYPE:
					addToken(tokens, PI_TYPE, "3.14159265358979", 16, idx);
				break;

				case E_TYPE:
					addToken(tokens, E_TYPE, "2.718281828", 11, idx);
				break;

				case DIVIDE:
					addToken(tokens, DIVIDE, inStr + idx, nextIdx-idx, idx);
				break;

				case AND:
					addToken(tokens, AND, inStr + idx, nextIdx-idx, idx);
				break;

				case OR:
					addToken(tokens, OR, inStr + idx, nextIdx-idx, idx);
				break;

				case SQRT:
					addToken(tokens, SQRT, inStr + idx, nextIdx-idx, idx);
				break;
			}
			idx = nextIdx;
		}
	}
}

/**
	Check if the current token is one of >, <, >=, <=, =, !=

	return TRUE if the token is one of above types
	otherwise return FALSE
*/	
int parserLogicOperator(const char *inStr, int length, int idx, int *type, int *outlen) {
	int result = FALSE;
	switch(inStr[idx]){
		case '>':
			if(inStr[idx+1] == '='){
				*type = GTE;
				*outlen = 2;
			}else{
				*type = GT;
				*outlen = 1;
			}
			result = TRUE;
			break;

		case '<':
			if(inStr[idx+1] == '='){
				*type = LTE;
				*outlen = 2;
			}else{
				*type = LT;
				*outlen = 1;
			}
			result = TRUE;
			break;

		case '!':
			if(inStr[idx+1] == '='){
				*type = NE;
				*outlen = 2;
				result = TRUE;
			}
			break;
	}
	return result;
}

/**
	Check Parenthese or prackets

	Return TRUE if it's a Parenthese or prackets
	otherwise return FALSE

	In case it returns TRUE, type will hold the actual type of the token
*/
int checkParenthesePrackets(char c, int *type) {
	int result = FALSE;

	switch(c) {

		case '(':
			*type = LPAREN;
			result = TRUE;
		break;
		
		case ')':
			*type = RPAREN;
			result = TRUE;
		break;
			
		case '[':
			*type = LPRACKET;
			result = TRUE;
		break;
			
		case ']':
			*type = RPRACKET;
			result = TRUE;
		break;
	}

	return result;
}

/**
	Check comma or semicolon.

	Return TRUE if it's a semiconlon or comma
	otherwise return FALSE

	In case it returns TRUE, type will hold the actual type of the token
*/
int checkCommaSemi(char c, int *type) {
	int result = FALSE;

	switch(c) {
		case ',':
			*type = COMMA;
			result = TRUE;
		break;
		
		case ';':
			*type = SEMI;
			result = TRUE;
		break;
	}
		
	return result;
}
	
/***********************************************************************/
/**
 * @see isNumericOperatorOREQ
 * @return TRUE if it's a numeric operator or equal sign
 */
int checkNumericOperator(const TokenList *tokens, const char *inStr, int length, int idx, int *type, int *textLength) {
	int result = FALSE;

	if(idx >= length)
		return result;

	*textLength = 1;

	switch(inStr[idx]) {
		case '+':
			result = TRUE;
			*type = PLUS;
		break;
			
		case '-':
			result = parseSubtractSign(tokens, inStr, length, idx, type, textLength);
		break;
			
		case '*':
			result = TRUE;
			*type = MULTIPLY;
		break;
			
		case '/':
			result = TRUE;
			*type = DIVIDE;
		break;
			
		case '^':
			result = TRUE;
			*type = POWER;
		break;
			
		case '=':
			if(idx==length-1 || inStr[idx+1] != '>'){
				result = TRUE;
				*type = EQ;
			} else if(inStr[idx+1] == '>'){
				result = TRUE;
				*type = IMPLY;
				*textLength = 2;
			}
		break;
	}
		
	return result;
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
int parseSubtractSign(const TokenList *tokens, const char *inStr, int length, int idx, int *type, int *outlen) {
	int result = FALSE;
	int k, floatingPoint;

	if(idx == (length-1) ) {
		//the minus sign is placed at the end of the string, it's a error
		return result;
	}
		
	if(inStr[idx+1] != '>') {

		/*
			Check if it's a negative number.
			If a minus sign is placed at the beginning of the input string or the previous token is in setLeadNegativeNumber then
			maybe you will get the a negative number.
		*/
		if (((idx == 0) || contains(tokens->list[tokens->size-1].type, setLeadNegativeNumber, LeadNegativeNumberSize))
				&& (isDigit(inStr[idx+1]))) {
				
			floatingPoint = FALSE;
			for(k = idx+1; k < length; k++){
				if(!isDigit(inStr[k])) {
					if(inStr[k] == '.') {
						//check if we got a floating point
						if(floatingPoint) { //<- the second floating point
							gErrorCode = ERROR_TOO_MANY_FLOATING_POINT;
							gErrorColumn = k;
							return FALSE;
						}
						floatingPoint = TRUE;
					} else {
						*type = NUMBER;
						*outlen = k - idx;
						return TRUE;
					}
				}
			}

			if( idx < k ) {
				*type = NUMBER;
				*outlen = k - idx;
				result = TRUE;
			}
			//////////////////////////////////////
		}else{
			*type = MINUS;
			*outlen = 1;
			result = TRUE;
		}
	}else{ //if(inputString.charAt(index+1)=='>'){
		*type = RARROW;
		*outlen = 2;
		result = TRUE;
	}
	return result;
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
	return TRUE
	otherwise return FALSE
*/
int isFunctionName(const char *inputString, int l, int index, int *outType, int *outlen) {
	int result = FALSE;	
	char c0, c1, c2;
		
	if(index+2 < l){
		c0 = inputString[index];
		c1 = inputString[index+1];
		c2 = inputString[index+2];
	}else
		return result;
			
	if( (index+5 < l) && c0=='c' && c1=='o' && c2 =='t' && 
				(inputString[index+3]=='a')	&& (inputString[index+4]=='n') ){
		(*outType) = COTAN;
		*outlen = 5;
		result = TRUE;
	}else if((index+4 < l) && (c0=='s' && c1=='q' && c2=='r' && inputString[index+3]=='t' )){
		(*outType) = SQRT;
		*outlen = 4;
		result = TRUE;
	}else if((index+4 < l) && (c0=='a' && c1=='t' && c2=='a' && inputString[index+3]=='n')){
		(*outType) = ATAN;
		*outlen = 4;
		result = TRUE;
	}else if((index+4 < l) && (c0=='a' && c1=='s' && c2=='i' && inputString[index+3]=='n')){
		(*outType) = ASIN;
		*outlen = 4;
		result = TRUE;
	}else if((index+4 < l) && (c0=='a' && c1=='c' && c2=='o' && inputString[index+3]=='s')) {
		(*outType) = ACOS;
		*outlen = 4;
		result = TRUE;
	} else if((index+3 < l)&& (c0=='t' && c1=='a' && c2=='n')){
		(*outType) = TAN;
		*outlen = 3;
		result = TRUE;
	}else if((index+3 < l) && (c0=='s' && c1=='i' && c2=='n')){
		(*outType) = SIN;
		*outlen = 3;
		result = TRUE;
	}else if((index+3 < l) &&(c0=='c' && c1=='o' && c2=='s')){
		(*outType) = COS;
		*outlen = 3;
		result = TRUE;
	}else if((index+3 < l) && (c0=='l' && c1=='o' && c2=='g')) {
		(*outType) = LOG;
		*outlen = 3;
		result = TRUE;
	}else if(c0=='l' && c1=='n'){
		(*outType) = LN;
		*outlen = 2;
		result = TRUE;
	}
	
	/*
		The following token of a function name MUST be an open parenthese
		TODO: I'm not sure if we need to check this
	*/
	if( (result==TRUE) && ( (*outlen + index )>=0) && (inputString[(*outlen + index )]!='(') ) {
		result = FALSE;
	}

	return result;
}
	
/**
	A NAME is a single character but not 'e' and placed at the end of inputString OR
	followed by (){}[];+-* / , ; > < ! space
*/
int isAName(const char *inputString, int length, int index) {
	char cc = inputString[index];
	char nextC;

	if(cc == 'e') return FALSE;
		
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