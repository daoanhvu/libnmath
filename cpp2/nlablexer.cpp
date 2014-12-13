#include <cstdlib>
#include <string>

#include "StringUtil.h"
#include "nlablexer.h"

#ifdef _TARGET_HOST_ANDROID
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NLABLEXER"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

using namespace nmath;

const int NLabLexer::setLeadNegativeNumber[14] = { LPAREN, LPRACKET, SEMI, COMMA, AND, OR, LT, LTE, GT, GTE, EQ, NE, IMPLY, RARROW };

NLabLexer::NLabLexer() {
	mList = 0;
	mSize = 0;
	mInputString = 0;
	mInputLen = 0;
}

NLabLexer::~NLabLexer() {
}

void NLabLexer::reset() {
	mList = 0;
	mSize = 0;
	mInputString = 0;
	mInputLen = 0;
}

/**
	Add a token info into mList, size of the mList will be increased one

	if SUCCESS, return the size of token
	otherwise return a negative value
*/
int NLabLexer::addToken(int _type, const char *_text, char txtlen, int _col) {

	if(mSize >= mCapability) {
		return E_NOT_ENOUGH_PLACE;
	}

	mList[mSize].type = _type;
	mList[mSize].column = _col;

	/*
		IMPORTANT: If you want to change this, PLEASE change function common:getPriorityOfType(int type) also
	*/
	switch(_type){
		case OR:
			mList[mSize].priority = 1;
		break;
		
		case AND:
			mList[mSize].priority = 2;
		break;
		
		case LT:
		case GT:
		case LTE:
		case GTE:
			mList[mSize].priority = 3;
		break;
		
		case PLUS:
		case MINUS:
			mList[mSize].priority = 4;
		break;
		
		case MULTIPLY:
		case DIVIDE:
			mList[mSize].priority = 5;
		break;
		
		case POWER:
			mList[mSize].priority = 6;
		break;
		
		case NE:
			mList[mSize].priority = 7;
		break;
		
		default:
			mList[mSize].priority = 0;
	}
	
	mList[mSize].textLength = (char)((MAXTEXTLEN < txtlen)?MAXTEXTLEN:txtlen);
	memcpy(mList[mSize].text, _text, mList[mSize].textLength);

	mSize++;

	return mSize;
}

/**********************************************************************/

/*
	This method do the anlysis on input string
	@param inStr the input string that need to be analysis [IN]
	@param len the length of inStr [IN]
	@return the size of the token list to be used actually
*/
int NLabLexer::lexicalAnalysis(char *inStr, int len, int appended/*NOT USE*/, Token *tokens, int capability, int start) {
	int chCode, type, k = 0;
	int idx = 0;
	int nextIdx;
	int floatingPoint;

	errorColumn = -1;
	errorCode = NMATH_NO_ERROR;

	if( (appended==TRUE) && (mSize > 0) ) {
		idx = mList[mSize-1].column;
		mSize -= 1;
	}

	mList = tokens;
	mCapability = capability;
	mInputString = inStr;
	mInputLen = len;
	mSize = 0;

	while( idx < len ) {
		//LOGI(3, "index: %d, 0x%X(%d)", idx, mInputString[idx], (char)(mInputString[idx]));
		if( (mInputString[idx] & 0x80) != 0x80 ) {
			if( checkNumericOperator(idx, &type, &k )==TRUE ) {
				addToken(type, mInputString + idx, k, idx);
				idx += k;
			}else if( checkParenthesePrackets(mInputString[idx], &type) == TRUE ) {
				addToken(type, mInputString + idx, 1, idx);
				idx++;
			}else if( checkCommaSemi(mInputString[idx], &type) == TRUE ) {
				addToken(type, mInputString + idx, 1, idx);
				idx++;
			}else if(parserLogicOperator(idx, &type, &k ) == TRUE) {
				addToken(type, mInputString + idx, k, idx);
				idx += k;
			}else if(mInputString[idx] == ':' ) {
				if(mInputString[idx+1] == '-' ) {
					addToken(ELEMENT_OF, mInputString + idx, 2, idx);
					idx += 2;
				}else{ //ERROR: bad token found
					errorColumn = idx;
					errorCode = ERROR_BAD_TOKEN;
					return 0;
				}
			}else if(isDigit(mInputString[idx])) {
				floatingPoint = FALSE;
				for(k = idx+1; k < len; k++) {
					if(!isDigit(mInputString[k])) {
						if(mInputString[k] == '.') {
							//check if we got a floating point
							if(floatingPoint){ //<- ERROR: the second floating point
								errorColumn = k;
								errorCode = ERROR_TOO_MANY_FLOATING_POINT;
								return 0;
							}
							floatingPoint = TRUE;
						} else {
							addToken(NUMBER, mInputString + idx, k-idx, idx);
							idx = k;
							break;
						}
					}
				}
				if(idx < k){
					addToken(NUMBER, mInputString + idx, k-idx, idx);
					idx = k;
				}
			}else if( isFunctionName(idx, &type, &k ) == TRUE ) {
				addToken(type, mInputString + idx, k, idx);
				idx += k;
			}else if(idx>0 && (mInputString[idx-1]==' ') && (mInputString[idx]=='D') && (mInputString[idx+1]==':') ){
				addToken(DOMAIN_NOTATION, "DOMAIN_NOTATION", 14, idx);
				idx += 2;
			}else if( isAName(idx) ) {
				addToken(NAME, mInputString + idx, 1, idx);
				idx++;
			}else if(mInputString[idx]=='o' || mInputString[idx]=='O') {
				if(mInputString[idx+1]=='r' || mInputString[idx+1]=='R'){
					addToken(OR, mInputString + idx, 2, idx);
					idx += 2;
				}else{
					//TODO: maybe its a NAME
					idx++;
				}
				
			}else if(mInputString[idx]=='a' || mInputString[idx]=='A'){
				if(mInputString[idx+1]=='n' || mInputString[idx+1]=='N'){
					if(mInputString[idx+2]=='d' || mInputString[idx+2]=='D'){
						addToken(AND, mInputString + idx, 3, idx);
						idx += 3;
					}
				}else{
					//TODO: maybe its a NAME
					idx++;
				}
				
			}else if( (idx+1 < len ) && (mInputString[idx]=='p' || mInputString[idx]=='P') && (mInputString[idx+1]=='i' || mInputString[idx+1]=='I')
							&& ( (idx+1 == len-1) || !isLetter(mInputString[idx+2]) ) ) {
				addToken(PI_TYPE, "3.14159265358979", 16, idx);
				idx += 2;
			}else if(mInputString[idx]=='e' && ((idx==len-1) || !isLetter(mInputString[idx+1]))) {
				addToken(E_TYPE, "2.718281828", 11, idx);
				idx++;
			}else
				idx++;

		} else {

			chCode = nmath::getCharacter(mInputString, len, idx, &nextIdx, &errorCode);
			if (errorCode != NMATH_NO_ERROR)
				return 0;
			//LOGI(3, "UTF Code: (0x%X)%d", chCode, chCode);
			switch(chCode) {
				case PI_TYPE:
					addToken(PI_TYPE, "3.14159265358979", 16, idx);
				break;

				case E_TYPE:
					addToken(E_TYPE, "2.718281828", 11, idx);
				break;

				case DIVIDE:
					addToken(DIVIDE, mInputString + idx, nextIdx-idx, idx);
				break;

				case AND:
					addToken(AND, mInputString + idx, nextIdx-idx, idx);
				break;

				case OR:
					addToken(OR, mInputString + idx, nextIdx-idx, idx);
				break;

				case SQRT:
					addToken(SQRT, mInputString + idx, nextIdx-idx, idx);
				break;
			}
			idx = nextIdx;
		}
	}

	//we don't hold external resource when we finish the job here
	mList = 0;
	mCapability = 0;
	mInputString = 0;
	mInputLen = 0;

	return mSize;
}

/**
	Check if the current token is one of >, <, >=, <=, =, !=

	return TRUE if the token is one of above types
	otherwise return FALSE
*/	
int NLabLexer::parserLogicOperator(int idx, int *type, int *outlen) {
	int result = FALSE;
	switch(mInputString[idx]){
		case '>':
			if (mInputString[idx + 1] == '='){
				*type = GTE;
				*outlen = 2;
			}else{
				*type = GT;
				*outlen = 1;
			}
			result = TRUE;
			break;

		case '<':
			if (mInputString[idx + 1] == '='){
				*type = LTE;
				*outlen = 2;
			}else{
				*type = LT;
				*outlen = 1;
			}
			result = TRUE;
			break;

		case '!':
			if (mInputString[idx + 1] == '='){
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
int nmath::checkParenthesePrackets(char c, int *type) {
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
int nmath::checkCommaSemi(char c, int *type) {
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
int NLabLexer::checkNumericOperator(int idx, int *type, int *textLength) {
	int result = FALSE;

	if (idx >= mInputLen)
		return result;

	*textLength = 1;

	switch(mInputString[idx]) {
		case '+':
			result = TRUE;
			*type = PLUS;
		break;
			
		case '-':
			result = parseSubtractSign(idx, type, textLength);
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
			if (idx == mInputLen - 1 || mInputString[idx + 1] != '>'){
				result = TRUE;
				*type = EQ;
			}
			else if (mInputString[idx + 1] == '>'){
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
	@param	idx current position in inputString [IN]
 * Example
		-32.3		-a		->
 */
int NLabLexer::parseSubtractSign(int idx, int *type, int *outlen) {
	int result = FALSE;
	int k, floatingPoint;

	if (idx == (mInputLen - 1)) {
		//the minus sign is placed at the end of the string, it's a error
		return result;
	}
		
	if(mInputString[idx+1] != '>') {

		/*
			Check if it's a negative number.
			If a minus sign is placed at the beginning of the input string or the previous token is in setLeadNegativeNumber then
			maybe you will get the a negative number.
		*/
		if (((idx == 0) || contains(mList[mSize-1].type, setLeadNegativeNumber, LeadNegativeNumberSize))
			&& (isDigit(mInputString[idx + 1]))) {
				
			floatingPoint = FALSE;
			for (k = idx + 1; k < mInputLen; k++){
				if (!isDigit(mInputString[k])) {
					if (mInputString[k] == '.') {
						//check if we got a floating point
						if(floatingPoint) { //<- the second floating point
							errorCode = ERROR_TOO_MANY_FLOATING_POINT;
							errorColumn = k;
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
	
/**
	return TRUE
	otherwise return FALSE
*/
int NLabLexer::isFunctionName(int index, int *outType, int *outlen) {
	int result = FALSE;	
	char c0, c1, c2;
		
	if(index+2 < mInputLen){
		c0 = mInputString[index];
		c1 = mInputString[index + 1];
		c2 = mInputString[index + 2];
	}else
		return result;
			
	if ((index + 5 < mInputLen) && c0 == 'c' && c1 == 'o' && c2 == 't' &&
		(mInputString[index + 3] == 'a') && (mInputString[index + 4] == 'n')){
		(*outType) = COTAN;
		*outlen = 5;
		result = TRUE;
	}
	else if ((index + 4 < mInputLen) && (c0 == 's' && c1 == 'q' && c2 == 'r' && mInputString[index + 3] == 't')){
		(*outType) = SQRT;
		*outlen = 4;
		result = TRUE;
	}
	else if ((index + 4 < mInputLen) && (c0 == 'a' && c1 == 't' && c2 == 'a' && mInputString[index + 3] == 'n')){
		(*outType) = ATAN;
		*outlen = 4;
		result = TRUE;
	}
	else if ((index + 4 < mInputLen) && (c0 == 'a' && c1 == 's' && c2 == 'i' && mInputString[index + 3] == 'n')){
		(*outType) = ASIN;
		*outlen = 4;
		result = TRUE;
	}
	else if ((index + 4 < mInputLen) && (c0 == 'a' && c1 == 'c' && c2 == 'o' && mInputString[index + 3] == 's')) {
		(*outType) = ACOS;
		*outlen = 4;
		result = TRUE;
	}
	else if ((index + 3 < mInputLen) && (c0 == 't' && c1 == 'a' && c2 == 'n')){
		(*outType) = TAN;
		*outlen = 3;
		result = TRUE;
	}
	else if ((index + 3 < mInputLen) && (c0 == 's' && c1 == 'i' && c2 == 'n')){
		(*outType) = SIN;
		*outlen = 3;
		result = TRUE;
	}
	else if ((index + 3 < mInputLen) && (c0 == 'c' && c1 == 'o' && c2 == 's')){
		(*outType) = COS;
		*outlen = 3;
		result = TRUE;
	}
	else if ((index + 3 < mInputLen) && (c0 == 'l' && c1 == 'o' && c2 == 'g')) {
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
	if ((result == TRUE) && ((*outlen + index) >= 0) && (mInputString[(*outlen + index)] != '(')) {
		result = FALSE;
	}

	(*outlen)++;

	return result;
}
	
/**
	A NAME is a single character but not 'e' and placed at the end of inputString OR
	followed by (){}[];+-* / , ; > < ! space
*/
int NLabLexer::isAName(int index) {
	char cc = mInputString[index];
	char nextC;

	if(cc == 'e') return FALSE;
		
	if(( cc>= 'a' && cc<='z' ) || (cc>= 'A' && cc<='Z')){
		if ((index < mInputLen - 1)){
			nextC = mInputString[index + 1];
			if(nextC==' ' || nextC=='+' || nextC=='-' || nextC=='*' || nextC=='/' || nextC=='^' || nextC=='='
					|| nextC=='(' || nextC==')' || nextC=='[' || nextC==']' || nextC=='<' || nextC=='>'
					|| nextC=='!'|| nextC==','|| nextC==';')
				return TRUE;
		}else
			return TRUE;
	}
	return FALSE;
}