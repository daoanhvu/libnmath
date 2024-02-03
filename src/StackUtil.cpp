#include <cstdlib>
#include <iostream>
#include "StackUtil.h"

namespace nmath {

	int pushItem2Stack(Token ***st, int *top, int *allocLen, Token *item) {
		Token** tmp;
		if ((*top) >= ((*allocLen) - 1)) {
			(*allocLen) += INCLEN;
			tmp = (nmath::Token**)realloc(*st, sizeof(Token*) * (*allocLen));
			if (tmp == nullptr){
				return E_NOT_ENOUGH_MEMORY;
			}
			(*st) = tmp;
		}
		(*top)++;
		(*st)[(*top)] = item;

		return NMATH_NO_ERROR;
	}

	Token* popFromStack(Token **st, int *top) {
		Token *p;
		if (st == nullptr || ((*top) < 0))
			return nullptr;
		p = st[(*top)];
		st[(*top)] = nullptr;
		(*top)--;
		return p;
	}

	int clearStackWithoutFreeItem(Token **ls, int len) {
		int i;
		//Token *p;

		if(ls==nullptr)
			return 0;

		for(i=0; i<len; i++){
			//p = ls[i];
			ls[i] = nullptr;
		}
		return i;
	}
}