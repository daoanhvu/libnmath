#include <cstdlib>
#include <iostream>
#include "StackUtil.h"
#include "nmath_pool.h"

using namespace StackUtil;

int StackUtil::pushItem2Stack(nmath::Token ***st, int *top, int *allocLen, nmath::Token *item) {
	nmath::Token** tmp;
	if ((*top) >= ((*allocLen) - 1)){
		(*allocLen) += INCLEN;
		tmp = (nmath::Token**)realloc(*st, sizeof(nmath::Token*) * (*allocLen));
		if (tmp == NULL){
			return E_NOT_ENOUGH_MEMORY;
		}
		(*st) = tmp;
	}
	(*top)++;
	(*st)[(*top)] = item;

	return NMATH_NO_ERROR;
}

nmath::Token* StackUtil::popFromStack(nmath::Token **st, int *top) {
	nmath::Token *p;
	if (st == NULL || ((*top) < 0))
		return NULL;
	p = st[(*top)];
	st[(*top)] = NULL;
	(*top)--;
	return p;
}

void StackUtil::addFunction2Tree(nmath::NMASTList *t, nmath::Token * stItm) {
	nmath::NMAST *ast = NULL;
	// LOGI(2, "Type: %d (%s)", stItm->type, stItm->text);
	switch (stItm->type) {
	case PLUS:
		if (t->size > 1) {
			ast = nmath::getFromPool();
			ast->type = stItm->type;
			ast->priority = stItm->priority;
			ast->left = t->list[t->size - 2];
			ast->right = t->list[t->size - 1];
			if ((t->list[t->size - 2]) != NULL)
				(t->list[t->size - 2])->parent = ast;
			if ((t->list[t->size - 1]) != NULL)
				(t->list[t->size - 1])->parent = ast;

			t->list[t->size - 2] = ast;
			t->list[t->size - 1] = NULL;
			t->size--;
		}
		break;

	case MINUS:
		if (t->size == 1) {
			if ((t->list[0]) != NULL)
				(t->list[0])->sign = -1;
		}
		else {
			ast = nmath::getFromPool();
			ast->type = stItm->type;
			ast->priority = stItm->priority;
			ast->left = t->list[t->size - 2];
			ast->right = t->list[t->size - 1];
			if ((t->list[t->size - 2]) != NULL)
				(t->list[t->size - 2])->parent = ast;
			if ((t->list[t->size - 1]) != NULL)
				(t->list[t->size - 1])->parent = ast;

			t->list[t->size - 2] = ast;
			t->list[t->size - 1] = NULL;
			t->size--;
		}
		break;

	case MULTIPLY:
	case DIVIDE:
	case POWER:

	case LT:
	case GT:
	case LTE:
	case GTE:
	case AND:
	case OR:
		ast = nmath::getFromPool();
		ast->type = stItm->type;
		ast->priority = stItm->priority;
		ast->left = t->list[t->size - 2];
		ast->right = t->list[t->size - 1];
		if ((t->list[t->size - 2]) != NULL)
			(t->list[t->size - 2])->parent = ast;
		if ((t->list[t->size - 1]) != NULL)
			(t->list[t->size - 1])->parent = ast;

		t->list[t->size - 2] = ast;
		t->list[t->size - 1] = NULL;
		t->size--;
		break;

	case SIN:
	case COS:
	case TAN:
	case COTAN:
	case ASIN:
	case ACOS:
	case ATAN:
	case SQRT:
	case LN:
		ast = nmath::getFromPool();
		ast->type = stItm->type;
		ast->priority = stItm->priority;
		ast->right = t->list[t->size - 1];
		if ((t->list[t->size - 1]) != NULL)
			(t->list[t->size - 1])->parent = ast;

		t->list[t->size - 1] = ast;
		break;

	case LOG:
		ast = nmath::getFromPool();
		ast->type = stItm->type;
		ast->priority = stItm->priority;
		ast->left = t->list[t->size - 2];
		ast->right = t->list[t->size - 1];
		if ((t->list[t->size - 2]) != NULL)
			(t->list[t->size - 2])->parent = ast;
		if ((t->list[t->size - 1]) != NULL)
			(t->list[t->size - 1])->parent = ast;

		t->list[t->size - 2] = ast;
		t->list[t->size - 1] = NULL;
		t->size--;
		break;
	}
}

