#ifndef _STACKUTIL_H
#define _STACKUTIL_H

#include "common_data.h"
#include "nmath_pool.hpp"

namespace nmath {
	/*return errorCode*/
    int pushItem2Stack(Token ***st, int *top, int *allocLen, Token *item);
    Token* popFromStack(Token **st, int *top);
	int clearStackWithoutFreeItem(Token **ls, int len);

	template <typename T>
	void addFunction2Tree(std::vector<nmath::NMAST<T>* > &t, Token *stItm, NMASTPool<T> *pool) {
		NMAST<T> *ast = nullptr;
		// LOGI(2, "Type: %d (%s)", stItm->type, stItm->text);
		switch (stItm->type) {
			case PLUS:
				if (t.size() > 1) {
					ast = pool->get();
					ast->type = stItm->type;
					ast->text = "+";
					ast->priority = stItm->priority;
					ast->left = t[t.size() - 2];
					ast->right = t[t.size() - 1];
					if ((t[t.size() - 2]) != nullptr)
						(t[t.size() - 2])->parent = ast;
					if ((t[t.size() - 1]) != nullptr)
						(t[t.size() - 1])->parent = ast;

					t[t.size() - 2] = ast;
					t[t.size() - 1] = nullptr;
					t.pop_back();
				}
				break;

			case MINUS:
				if (t.size() == 1) {
					// This is because, in the postfix we have operand1 operand2 operator
					// in this case, the operator is MINUS, it means operand1 - operand2 and if we miss one operand
					// we assume that missing operand is always operand1 then we have -operand2
					if ((t[0]) != nullptr)
						(t[0])->sign = -1;
				}
				else {
					ast = pool->get();
					ast->type = stItm->type;
					ast->text = "-";
					ast->priority = stItm->priority;
					ast->left = t[t.size() - 2];
					ast->right = t[t.size() - 1];
					if ((t[t.size() - 2]) != nullptr)
						(t[t.size() - 2])->parent = ast;
					if ((t[t.size() - 1]) != nullptr)
						(t[t.size() - 1])->parent = ast;

					t[t.size() - 2] = ast;
					t[t.size() - 1] = nullptr;
					t.pop_back();
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
				ast = pool->get();
				ast->type = stItm->type;
				ast->text = stItm->text;
				ast->priority = stItm->priority;
				ast->left = t[t.size() - 2];
				ast->right = t[t.size() - 1];
				if ((t[t.size() - 2]) != nullptr)
					(t[t.size() - 2])->parent = ast;
				if ((t[t.size() - 1]) != nullptr)
					(t[t.size() - 1])->parent = ast;

				t[t.size() - 2] = ast;
				t[t.size() - 1] = nullptr;
				t.pop_back();
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
				ast = pool->get();
				ast->type = stItm->type;
				ast->text = stItm->text;
				ast->priority = stItm->priority;
				ast->right = t[t.size() - 1];
				if ((t[t.size() - 1]) != nullptr)
					(t[t.size() - 1])->parent = ast;

				t[t.size() - 1] = ast;
				break;

			case LOG:
				ast = pool->get();
				ast->type = stItm->type;
				ast->text = stItm->text;
				ast->priority = stItm->priority;
				ast->left = t[t.size() - 2];
				ast->right = t[t.size() - 1];
				if ((t[t.size() - 2]) != nullptr)
					(t[t.size() - 2])->parent = ast;
				if ((t[t.size() - 1]) != nullptr)
					(t[t.size() - 1])->parent = ast;

				t[t.size() - 2] = ast;
				t[t.size() - 1] = nullptr;
				t.pop_back();
				break;
		}
	}

}

#endif
