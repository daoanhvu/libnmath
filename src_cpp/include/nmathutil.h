#ifndef _NMATHUTIL_H_
#define _NMATHUTIL_H_

#include "common.hpp"

typedef struct tagTextToken TextToken;
typedef struct tagTextTokenList TextTokenList;

struct tagTextToken {
	char text[512];
	int length;
	char level;

	struct tagTextToken *next;
};

struct tagTextTokenList {
	struct tagTextToken *head;
	struct tagTextToken *tail;
};

#endif