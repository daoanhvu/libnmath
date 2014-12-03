#ifndef _STACKUTIL_H
#define _STACKUTIL_H

#include "common.h"

namespace StackUtil {
	/*return errorCode*/
	int pushItem2Stack(Token ***st, int *top, int *allocLen, Token *item);
	Token* popFromStack(Token **st, int *top);
}

#endif

