#ifdef _WIN32
	#include <Windows.h>
#else
	
#endif

#include "nmath_pool.h"

#define POOL_CAPACITY 32

#ifdef _WIN32
HANDLE poolMutex;
#else
pthread_mutex_t poolMutex;
#endif
bool mutexInitialized = false;

int gPoolSize = 0;
nmath::NMAST* AST_POOL[POOL_CAPACITY];

void nmath::initNMASTPool() {
	if(mutexInitialized)
		return;
		
#ifdef _WIN32
	poolMutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
#else
	pthread_mutex_init(&poolMutex, NULL);
#endif
	mutexInitialized = true;
}

void nmath::releaseNMASTPool() {
	clearPool();
	
	if(!mutexInitialized)
		return;
		
#ifdef _WIN32
    CloseHandle(poolMutex);
	poolMutex = 0;
#else
	pthread_mutex_destroy(&poolMutex);
#endif
	mutexInitialized = false;
}

/** Pool operations */
nmath::NMAST* nmath::getFromPool() {
	nmath::NMAST *node;
	
#ifdef _WIN32
	WaitForSingleObject(poolMutex,INFINITE);
#else
	pthread_mutex_lock(&poolMutex);
#endif
	if( gPoolSize>0 ) {
		node = AST_POOL[gPoolSize-1];
		AST_POOL[gPoolSize-1] = NULL;
		gPoolSize--;
	} else {
		node = (NMAST*)malloc(sizeof(NMAST));
		node->valueType = TYPE_FLOATING_POINT;
		node->sign = 1;
		node->variable = 0;
		node->left = node->right = node->parent = NULL;
		node->value = 0;
		node->type = NUMBER;
		node->priority = 0;
		//node->frValue.numerator = 0;
		//node->frValue.denomerator = 1;
	}
#ifdef _WIN32
#else
	pthread_mutex_unlock(&poolMutex);
#endif
	return node;
}

void nmath::putIntoPool(nmath::NMAST *node) {
	nmath::NMAST *p;

	if(node == NULL) return;

	if(node->left != NULL)
		putIntoPool(node->left);

	if(node->right != NULL)
		putIntoPool(node->right);

	if(node->parent != NULL) {
		p = node->parent;
		if(p->left == node)
			p->left = NULL;
		else if(p->right == node)
			p->right = NULL;

		node->parent = NULL;
	}

	node->valueType = TYPE_FLOATING_POINT;
	node->sign = 1;
	node->variable = 0;
	node->left = node->right = node->parent = NULL;
	node->value = 0;
	node->type = NUMBER;
	node->priority = 0;
	//node->frValue.numerator = 0;
	//node->frValue.denomerator = 1;
#ifdef _WIN32
	WaitForSingleObject(poolMutex,INFINITE);
#else
	pthread_mutex_lock(&poolMutex);
#endif
	if( gPoolSize < POOL_CAPACITY ) {
		AST_POOL[gPoolSize] = node;
		gPoolSize++;
#ifdef _WIN32
#else
	pthread_mutex_unlock(&poolMutex);
#endif
		return;
	}
#ifdef _WIN32
#else
	pthread_mutex_unlock(&poolMutex);
#endif

	free(node);
}

void nmath::clearPool() {
	int i;
#ifdef _WIN32
	WaitForSingleObject(poolMutex,INFINITE);
#else
	pthread_mutex_lock(&poolMutex);
#endif
	if(gPoolSize > 0) {
		for(i=0; i<gPoolSize; i++) {
			free(AST_POOL[i]);
			AST_POOL[i] = NULL;
		}
		gPoolSize = 0;
	}
#ifdef _WIN32
#else
	pthread_mutex_unlock(&poolMutex);
#endif
}