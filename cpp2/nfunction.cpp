#include "nfunction.h"
#include "nlablexer.h"
#include "nlabparser.h"

using namespace nmath;

NFunction::NFunction() {
	
}

NFunction::~NFunction() {
	release();
}

void NFunction::parse() {
	TokenList lst;
	int result;
	
	lst.loggedSize = len;
	lst.list = new Token[lst.loggedSize];
	lst.size = 0;

	/* build the tokens list from the input string */
	
	lexicalAnalysisUTF8(str, len, &lst, 0);
	
	if(errorCode == NMATH_NO_ERROR ) {
		result = parseFunctionExpression(&lst, outF);
	}
	delete lst.list;	
}
		
void NFunction::release() {
	
}