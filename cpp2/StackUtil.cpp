#include <cstdlib>
#include <iostream>
#include "StackUtil.h"

using namespace StackUtil;

int StackUtil::pushItem2Stack(Token ***st, int *top, int *allocLen, Token *item) {
	Token** tmp;
	if ((*top) >= ((*allocLen) - 1)){
		(*allocLen) += INCLEN;
		tmp = (Token**)realloc(*st, sizeof(Token*) * (*allocLen));
		if (tmp == NULL){
			return E_NOT_ENOUGH_MEMORY;
		}
		(*st) = tmp;
	}
	(*top)++;
	(*st)[(*top)] = item;

	return NMATH_NO_ERROR;
}

Token* StackUtil::popFromStack(Token **st, int *top) {
	Token *p;
	if (st == NULL || ((*top) < 0))
		return NULL;
	p = st[(*top)];
	st[(*top)] = NULL;
	(*top)--;
	return p;
}
