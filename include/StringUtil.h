#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#include "common_data.h"
#include <cstdarg>

namespace nmath {
	int getCharacter(const char *str, int length, int index, int *nextIdx, int *errorCode);
	int isDigit(char c);
	bool isLetter(char c);
	bool isOneOf(char c, int numCh, ...);
	int isInArray(char *vars, char c);
	void getOperatorChar(int operatorType, char *opCh);
}

#endif

