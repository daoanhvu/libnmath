#ifndef _NLABPARSER_H
#define _NLABPARSER_H

#include "common.h"

/**
	access right: private
	@param outF a NOT-NULL Function pointer
	@return errorCode
*/	
short parseFunction(const char *str, short len, Function *outF);
void parseExpression(TokenList *tokens, int *start, Function *f);

#endif