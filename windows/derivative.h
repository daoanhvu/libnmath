/**
 *  @author: Dao Anh Vu
 * */
 
#ifndef ___DERIVATIVE
#define ___DERIVATIVE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "funct.h"

#include <Windows.h>
#include <process.h>

/**
 * ToDo
 * 	Must test cloneTree build a new tree base on existed tree
 * */
 
unsigned int __stdcall derivative(void *);
int isContainVar(TNode *t, char x);

#endif
