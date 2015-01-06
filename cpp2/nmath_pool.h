#ifndef _NMAST_POOL_H
#define _NMAST_POOL_H

#include "common.h"

namespace nmath {
/* 
	I use a pool to store AST node to reuse them later 
	this reduces the number of allocation operation so it speed up the app
*/
void initNMASTPool();
void releaseNMASTPool();
NMAST* getFromPool();
void putIntoPool(NMAST *ast);
void clearPool();
}

#endif