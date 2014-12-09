#include "criteria.h"
#include "SimpleCriteria.h"
#include "compositecriteria.h"

using namespace nmath;

Criteria::Criteria() {
}

Criteria* nmath::buildTree(const NMAST *ast) {

	Criteria *out = 0;
	Criteria *left, *right;
	switch (ast->type) {
		case AND:
		case OR:
			out = new CompositeCriteria();
			((CompositeCriteria*)out)->setOperator(ast->type);
			left = buildTree(ast->left);
			((CompositeCriteria*)out)->add(left);
			right = buildTree(ast->right);
			((CompositeCriteria*)out)->add(right);
			break;

		case LT:
			out = new SimpleCriteria(GT_LT, ast->left->variable, 0, ast->right->value, true, false );
			break;

		case LTE:
			out = new SimpleCriteria(GT_LTE, ast->left->variable, 0, ast->right->value, true, false);
			break;

		case GT:
			out = new SimpleCriteria(GT_LT, ast->left->variable, ast->right->value, 0, false, true);
			break;

		case GTE:
			out = new SimpleCriteria(GTE_LT, ast->left->variable, ast->right->value, 0, false, true);
			break;

		case GT_LT:
			break;

		case GT_LTE:
			break;

		case GTE_LT:
			break;

		case GTE_LTE:
			break;
	}

	return out;
}