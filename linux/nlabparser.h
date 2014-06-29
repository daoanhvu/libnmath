#ifndef _NLABPARSER_H
#define _NLABPARSER_H

#include "common.h"

/**
	Parse the input string in object f to NMAST tree
	@return
		0 if everything ok otherwise it returns a value != 0
		in case error occurs, idxE will hold the position where error comes from.
*/
int parseFunction(Function *f, int *idxE);

#endif