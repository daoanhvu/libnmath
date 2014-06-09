#ifndef _NLABPARSER_H
#define _NLABPARSER_H

#include "common.h"

void clearTree(NMAST **prf);
int parseFunct(TokenList *tokens, Function *f, int *idxE);

#endif