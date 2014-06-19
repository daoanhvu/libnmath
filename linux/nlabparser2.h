#ifndef _NLABPARSER2_H
#define _NLABPARSER2_H

#include "common.h"
int getErrorColum();
int getErrorCode();
Function* parseFunctionExpression(TokenList *tokens);
#endif