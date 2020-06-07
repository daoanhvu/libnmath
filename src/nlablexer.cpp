#include <cstdlib>
#include <cstring>

#include "nlablexer.h"
#include "internal_common.h"
#include "common.hpp"
#include "utf8.h"

using namespace nmath;

const int NLabLexer::setLeadNegativeNumber[14] = { LPAREN, LPRACKET, SEMI, COMMA, AND, OR, LT, LTE, GT, GTE,
												   EQ, NE, IMPLY, RARROW };

/**
	Add a token info into mList, size of the mList will be increased one

	if SUCCESS, return the size of token
	otherwise return a negative value
*/
Token* NLabLexer::addToken(int _type, const char *_text, int len, int _col) {

    auto *tk = new Token;
    tk->type = _type;
    tk->column = _col;

	/*
		IMPORTANT: If you want to change this, PLEASE change function common:getPriorityOfType(int type) also
	*/
	switch(_type){
		case OR:
			tk->priority = 1;
		break;
		
		case AND:
			tk->priority = 2;
		break;
		
		case LT:
		case GT:
		case LTE:
		case GTE:
		    tk->priority = 3;
		break;
		
		case PLUS:
		case MINUS:
            tk->priority = 4;
		break;
		
		case MULTIPLY:
		case DIVIDE:
            tk->priority = 5;
		break;
		
		case POWER:
            tk->priority = 6;
		break;
		
		case NE:
            tk->priority = 7;
		break;
		
		default:
            tk->priority = 0;
	}
	tk->textLength = (unsigned char)((MAXTEXTLEN < len)?MAXTEXTLEN:len);
	memcpy(tk->text, _text, tk->textLength);
	tk->text[tk->textLength] = '\0';
	return tk;
}

/**********************************************************************/

/**
  This method do the analysis on input string
  @param inStr     [IN] the input string that need to be analysis [IN]
  @param len       [IN] the length of inStr [IN]
  @param appended  [IN]
  @param start     [IN] The position where starting the anlysis
  @param tokens    [OUT] 
  @param lastMeanIdx [OUT]

  @return the size of the token list to be used actually
*/
size_t NLabLexer::lexicalAnalysis(const char *inStr, int len,
          bool appended, int start, std::vector<Token*> &tokens, int *lastMeanIdx) {
  int chCode, type, k = 0;
  int idx = start;
  int nextIdx;
  bool floatingPoint;

  errorColumn = -1;
  errorCode = NMATH_NO_ERROR;
	
	while( idx < len ) {

	    /* if character at [idx] is unicode (UTF-8) then it's a 2-bytes char and
	     * the first byte will be 0x80
	     * */
		if( (inStr[idx] & 0x80) != 0x80 ) {
			if( checkNumericOperator(inStr, len, idx, &type, &k, tokens) ) {
				tokens.push_back(addToken(type, inStr + idx, k, idx));
				idx += k;
			}else if( checkParenthesePrackets(inStr[idx], &type) ) {
                tokens.push_back(addToken(type, inStr + idx, 1, idx));
				idx++;
			}else if( checkCommaSemi(inStr[idx], &type) ) {
                tokens.push_back(addToken(type, inStr + idx, 1, idx));
				idx++;
			}else if(parserLogicOperator(inStr, idx, &type, &k )) {
                tokens.push_back(addToken(type, inStr + idx, k, idx));
				idx += k;
			}else if(inStr[idx] == ':' ) {
				if(inStr[idx+1] == '-' ) {
                    tokens.push_back(addToken(ELEMENT_OF, inStr + idx, 2, idx));
					idx += 2;
				}else{ //ERROR: bad token found
					errorColumn = idx;
					errorCode = ERROR_BAD_TOKEN;
                    *lastMeanIdx = idx - 1;
					return 0;
				}
			}else if(isDigit(inStr[idx])) {
				floatingPoint = false;
				for(k = idx+1; k < len; k++) {
					if(!isDigit(inStr[k])) {
						if(inStr[k] == '.') {
							//check if we got a floating point
							if(floatingPoint){ //<- ERROR: the second floating point
								errorColumn = k;
								errorCode = ERROR_TOO_MANY_FLOATING_POINT;
                                *lastMeanIdx = idx - 1;
								return 0;
							}
							floatingPoint = true;
						} else {
							tokens.push_back(addToken(NUMBER, inStr + idx, k-idx, idx));
							idx = k;
							break;
						}
					}
				}
				if(idx < k){
					tokens.push_back(addToken(NUMBER, inStr + idx, k-idx, idx));
					idx = k;
				}
			}else if( isFunctionName(inStr, len, idx, &type, &k ) ) {
                tokens.push_back(addToken(type, inStr + idx, k, idx));
				idx += k;
			}else if(idx>0 && (inStr[idx-1]==' ') && (inStr[idx]=='D') && (inStr[idx+1]==':') ){
                tokens.push_back(addToken(DOMAIN_NOTATION, "DOMAIN_NOTATION", 14, idx));
				idx += 2;
			}else if( isAName(inStr, len, idx, &k) ) {
				tokens.push_back(addToken(NAME, inStr + idx, k, idx));
				idx += k;
			}else if( (idx+1 < len ) && (inStr[idx]=='p' || inStr[idx]=='P') && (inStr[idx+1]=='i' || inStr[idx+1]=='I')
							&& ( (idx+1 == len-1) || !isASCIILetter(inStr[idx+2]) ) ) {
                tokens.push_back(addToken(PI_TYPE, "3.14159265358979", 16, idx));
				idx += 2;
			}else if(inStr[idx]=='e' && ((idx==len-1) || !isASCIILetter(inStr[idx+1]))) {
                tokens.push_back(addToken(E_TYPE, "2.718281828", 11, idx));
				idx++;
			}else
				idx++;

		} else {

			chCode = getCharacter(inStr, len, idx, &nextIdx, &errorCode);
			if (errorCode != NMATH_NO_ERROR)
				return 0;
			//LOGI(3, "UTF Code: (0x%X)%d", chCode, chCode);
			switch(chCode) {
				case PI_TYPE:
                    tokens.push_back(addToken(PI_TYPE, "3.14159265358979", 16, idx));
				break;

				case E_TYPE:
                    tokens.push_back(addToken(E_TYPE, "2.718281828", 11, idx));
				break;

				case DIVIDE:
                    tokens.push_back(addToken(DIVIDE, inStr + idx, nextIdx-idx, idx));
				break;

				case AND:
                    tokens.push_back(addToken(AND, inStr + idx, nextIdx-idx, idx));
				break;

				case OR:
                    tokens.push_back(addToken(OR, inStr + idx, nextIdx-idx, idx));
				break;

				case SQRT:
                    tokens.push_back(addToken(SQRT, inStr + idx, nextIdx-idx, idx));
				break;

                default:
                    break;
			}
			idx = nextIdx;
		}
	}

	return tokens.size();
}

size_t NLabLexer::lexicalAnalysis(std::string inStr,
		bool appended, 
		int start, 
		std::vector<Token*> &tokens, 
		int *lastMeanIdx) {

	return lexicalAnalysis(inStr.c_str(), inStr.length(), appended, start, tokens, lastMeanIdx);

}

/**
	Check if the current token is one of >, <, >=, <=, =, !=, and, or

	return TRUE if the token is one of above types
	otherwise return FALSE
*/	
bool NLabLexer::parserLogicOperator(const char *inStr, int idx, int *type, int *outlen) {
	bool result = false;
	switch(inStr[idx]){
		case '>':
			if (inStr[idx + 1] == '='){
				*type = GTE;
				*outlen = 2;
			}else{
				*type = GT;
				*outlen = 1;
			}
			result = true;
			break;

		case '<':
			if (inStr[idx + 1] == '='){
				*type = LTE;
				*outlen = 2;
			}else{
				*type = LT;
				*outlen = 1;
			}
			result = true;
			break;

		case '!':
			if (inStr[idx + 1] == '='){
				*type = NE;
				*outlen = 2;
				result = true;
			}
			break;

	    case 'a':
	    case 'A':
            if(inStr[idx+1]=='n' || inStr[idx+1]=='N') {
                if( (inStr[idx+2]=='d' || inStr[idx+2]=='D') &&
                        (inStr[idx+3]==' ' || inStr[idx+3]=='+' || inStr[idx+3]=='-' || inStr[idx+3]=='*'
                        || inStr[idx+3]=='/' || inStr[idx+3]=='^' || inStr[idx+3]=='(' || inStr[idx+3]==')'
                        || inStr[idx+3]=='[' || inStr[idx+3]==']' || inStr[idx+3]=='{' || inStr[idx+3]=='}') ) {
                    *type = AND;
                    *outlen = 3;
                    result = true;
                }
            }
	        break;
	    case 'o':
	    case 'O':
            if( (inStr[idx+1]=='r' || inStr[idx+1]=='R') &&
                (inStr[idx+3]==' ' || inStr[idx+3]=='+' || inStr[idx+3]=='-' || inStr[idx+3]=='*'
                 || inStr[idx+3]=='/' || inStr[idx+3]=='^' || inStr[idx+3]=='(' || inStr[idx+3]==')'
                 || inStr[idx+3]=='[' || inStr[idx+3]==']' || inStr[idx+3]=='{' || inStr[idx+3]=='}') ) {
                *type = OR;
                *outlen = 2;
                result = true;
            }
	        break;

        default:
            return false;
	}
	return result;
}

/**
	Check comma or semicolon.

	Return TRUE if it's a semiconlon or comma
	otherwise return FALSE

	In case it returns TRUE, type will hold the actual type of the token
*/
bool nmath::checkCommaSemi(char c, int *type) {
	switch(c) {
		case ',':
			*type = COMMA;
			return true;
		
		case ';':
			*type = SEMI;
			return true;

		default:
			return false;
	}
}
	
/***********************************************************************/
/**
 * @see isNumericOperatorOREQ
 * @return TRUE if it's a numeric operator or equal sign
 */
bool NLabLexer::checkNumericOperator(const char *mInputString, int mInputLen, int idx, int *type, int *textLength, vector<Token*> list) {
	bool result = false;

	if (idx >= mInputLen)
		return result;

	*textLength = 1;

	switch(mInputString[idx]) {
		case '+':
			result = true;
			*type = PLUS;
		break;
			
		case '-':
			result = parseSubtractSign(mInputString, mInputLen, idx, type, std::move(list), textLength);
		break;
			
		case '*':
			result = true;
			*type = MULTIPLY;
		break;
			
		case '/':
			result = true;
			*type = DIVIDE;
		break;
			
		case '^':
			result = true;
			*type = POWER;
		break;
			
		case '=':
			if (idx == mInputLen - 1 || mInputString[idx + 1] != '>'){
				result = true;
				*type = EQ;
			}
			else if (mInputString[idx + 1] == '>'){
				result = true;
				*type = IMPLY;
				*textLength = 2;
			}
		break;

		default:
			result = false;
	}
		
	return result;
}
	
bool NLabLexer::isNumericOperatorOREQ(char c){
	switch(c) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
		case '=':
			return true;
        default:
            return false;
	}
}
	/***********************************************************************/
	
/**
 * Just call this routine if character at currentPos is a minus sign
	@param	idx current position in inputString [IN]
 * Example
		-32.3		-a		->
 */
bool NLabLexer::parseSubtractSign(const char *mInputString, int mInputLen, int idx, int *type,
		vector<Token*> mList, int *outlen) {
	bool result = false;
	int k, floatingPoint;
	size_t mSize = mList.size();
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
		if (((idx == 0) || nmath::contains<int>(mList[mSize-1]->type, setLeadNegativeNumber, LeadNegativeNumberSize))
			&& (isDigit(mInputString[idx + 1]))) {
				
			floatingPoint = false;
			for (k = idx + 1; k < mInputLen; k++){
				if (!isDigit(mInputString[k])) {
					if (mInputString[k] == '.') {
						//check if we got a floating point
						if(floatingPoint) { //<- the second floating point
							errorCode = ERROR_TOO_MANY_FLOATING_POINT;
							errorColumn = k;
							return false;
						}
						floatingPoint = true;
					} else {
						*type = NUMBER;
						*outlen = k - idx;
						return true;
					}
				}
			}

			if( idx < k ) {
				*type = NUMBER;
				*outlen = k - idx;
				result = true;
			}
			//////////////////////////////////////
		} else {
			*type = MINUS;
			*outlen = 1;
			result = true;
		}
	} else { //if(inputString.charAt(index+1)=='>'){
		*type = RARROW;
		*outlen = 2;
		result = true;
	}
	return result;
}
	
bool NLabLexer::isLogicOperator(char c){
	switch(c){
		case '>':
		case '<':
		case '!':
			return true;
        default:
            return false;
	}
}
	
/**
	return TRUE
	otherwise return FALSE
*/
bool NLabLexer::isFunctionName(const char *mInputString, int mInputLen, int index, int *outType, int *outlen) {
	bool result = false;	
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
		result = true;
	}
	else if ((index + 4 < mInputLen) && (c0 == 's' && c1 == 'q' && c2 == 'r' && mInputString[index + 3] == 't')){
		(*outType) = SQRT;
		*outlen = 4;
		result = true;
	}
	else if ((index + 4 < mInputLen) && (c0 == 'a' && c1 == 't' && c2 == 'a' && mInputString[index + 3] == 'n')){
		(*outType) = ATAN;
		*outlen = 4;
		result = true;
	}
	else if ((index + 4 < mInputLen) && (c0 == 'a' && c1 == 's' && c2 == 'i' && mInputString[index + 3] == 'n')){
		(*outType) = ASIN;
		*outlen = 4;
		result = true;
	}
	else if ((index + 4 < mInputLen) && (c0 == 'a' && c1 == 'c' && c2 == 'o' && mInputString[index + 3] == 's')) {
		(*outType) = ACOS;
		*outlen = 4;
		result = true;
	}
	else if ((index + 3 < mInputLen) && (c0 == 'a' && c1 == 'b' && c2 == 's')){
		(*outType) = ABS;
		*outlen = 3;
		result = true;
	}
	else if ((index + 3 < mInputLen) && (c0 == 't' && c1 == 'a' && c2 == 'n')){
		(*outType) = TAN;
		*outlen = 3;
		result = true;
	}
	else if ((index + 3 < mInputLen) && (c0 == 's' && c1 == 'i' && c2 == 'n')){
		(*outType) = SIN;
		*outlen = 3;
		result = true;
	}
	else if ((index + 3 < mInputLen) && (c0 == 'c' && c1 == 'o' && c2 == 's')){
		(*outType) = COS;
		*outlen = 3;
		result = true;
	}
	else if ((index + 3 < mInputLen) && (c0 == 'l' && c1 == 'o' && c2 == 'g')) {
		(*outType) = LOG;
		*outlen = 3;
		result = true;
	}else if(c0=='l' && c1=='n'){
		(*outType) = LN;
		*outlen = 2;
		result = true;
	}
	
	/*
		The following token of a function name MUST be an open parenthese
		TODO: I'm not sure if we need to check this
	*/
	if (result && ((*outlen + index) >= 0) && (mInputString[(*outlen + index)] != '(')) {
		result = false;
	}

	(*outlen)++;

	return result;
}
	
/**
	A NAME is a substring but not 'e', 'and', 'or' and placed at the end of inputString OR
	followed by (){}[];+-* / , ; > < ! space

    @param tlen [OUT] In case of return true, this will be the length of the token
*/
bool NLabLexer::isAName(const char* inStr, int inputLen, int idx, int *tlen) {
	char cc = inStr[idx];
	if(( cc>= 'a' && cc<='z' ) || (cc>= 'A' && cc<='Z')) {
	    int k = idx + 1;

	    while(k < inputLen && !isDelimiterChar(inStr[k])) {
	    	k++;
	    }

	    if( (k == idx+1) && cc == 'e' ) {
	        return false;
	    }

        if( (k == idx+2)
            && (cc == 'o' || cc=='O' )
            && (inStr[idx+1]=='r' || inStr[idx+1]=='R' )) {
            return false;
        }

        *tlen = k - idx;

        return !((k == idx + 3)
                 && (cc == 'a' || cc=='A' )
                 && (inStr[idx+1]=='n' || inStr[idx+1]=='N' )
                 && (inStr[idx+2]=='d' || inStr[idx+2]=='D' ));

    }
	return false;
}

bool NLabLexer::isDelimiterChar(char nextC) {
	return nextC == ' ' || nextC == '+' || nextC == '-' || nextC == '*' || nextC == '/' || nextC == '^' || nextC == '='
		   || nextC=='(' || nextC==')' || nextC=='[' || nextC==']' || nextC=='<' || nextC=='>'
		   || nextC=='!' || nextC==',' || nextC==';';

}
