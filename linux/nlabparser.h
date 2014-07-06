#ifndef _NLABPARSER_H
#define _NLABPARSER_H

#include "common.h"

/**
	access right: private
	@param outF a NOT-NULL Function pointer
	@return errorCode
*/	
int parseFunction(const char *str, int len, Function *outF);

#endif