#ifndef _STACKUTIL_H
#define _STACKUTIL_H

#include "common.h"

namespace StackUtil {
	/*return errorCode*/
	int pushItem2Stack(nmath::Token ***st, int *top, int *allocLen, nmath::Token *item);
	nmath::Token* popFromStack(nmath::Token **st, int *top);
	void addFunction2Tree(nmath::NMASTList *t, nmath::Token * stItm);
}

#endif

