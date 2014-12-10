#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#include "common.h"

namespace nmath {
	int getCharacter(const char *str, int length, int index, int *nextIdx, int *errorCode);
	int isDigit(char c);
	int isInArray(char *vars, char c);
	void getOperatorChar(int operatorType, char *opCh);
	void toString(const NMAST *t, char *str, int *curpos, int len);
}

#endif

