#include "nfunction.h"
#include "../linux/nlabparser.h"

NFunction::NFunction() {
	
}

NFunction::~NFunction() {
	release();
}

void NFunction::parse() {
	TokenList lst;
	int result;
	
	lst.loggedSize = len;
	lst.list = (Token*)malloc(sizeof(Token) * lst.loggedSize);
	lst.size = 0;

	/* build the tokens list from the input string */
	
	lexicalAnalysisUTF8(str, len, &lst);
	
	if( gErrorCode == NMATH_NO_ERROR ) {
		result = parseFunctionExpression(&lst, outF);
	}
	free(lst.list);
	return result;
}
		
void NFunction::release() {
	
}