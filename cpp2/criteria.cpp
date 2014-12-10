#include "criteria.h"
#include "SimpleCriteria.h"
#include "compositecriteria.h"

using namespace nmath;

Criteria::Criteria() {
}

/*
Criteria* nmath::buildSameVariableCriteria(const NMAST *left, const NMAST *right){
	SimpleCriteria* out;
	
	if( (left->type == LT) && (right->type == LT) ){
		
	} else if( (left->type == LT) && (right->type == LTE) ){
	
	} else if( (left->type == LT) && (right->type == LTE) ){
	
	}
	
	return out;
}
*/

Criteria* nmath::buildCriteria(const NMAST *ast) {

	Criteria *out = 0;
	Criteria *left, *right;
	SimpleCriteria *sc;
	double v1, v2;
	switch (ast->type) {
		case AND:
			if(isComparationOperator(ast->left->type) && isComparationOperator(ast->right->type)) {
				if( ast->left->variable == ast->right->variable){
					switch(ast->left->type) {
						case LT:
							out = new SimpleCriteria(GT_LT, ast->left->variable, 0, ast->right->value, true, false );
							sc = (SimpleCriteria*)out;
							switch(ast->right->type) {
								case LT:
									if(ast->left->value >= ast->right->value) {
										sc->setRightValue(ast->left->value);
									}
									break;

								case LTE:
									if(ast->left->value >= ast->right->value) {
										sc->setType(GT_LTE);
										sc->setRightValue(ast->left->value);
									}	
									break;

								case GT:
									sc->setLeftValue(ast->left->value);
									sc->setLeftInfinity(false);
									break;

								case GTE:
									sc->setType(GTE_LT);
									sc->setLeftValue(ast->left->value);
									sc->setLeftInfinity(false);
									break;
							}
							break;

						case LTE:
							out = new SimpleCriteria(GT_LTE, ast->left->variable, 0, ast->right->value, true, false);
							break;

						case GT:
							out = new SimpleCriteria(GT_LT, ast->left->variable, ast->left->value, 0, false, true);
							sc = (SimpleCriteria*)out;
							switch(ast->right->type) {
								case LT:
									if(ast->left->value < ast->right->value) {
										return 0;
									}
									
									sc->setRightValue(ast->right->value);
									sc->setRightInfinity(false);
									break;

								case LTE:
									if(ast->left->value > ast->right->value) {
										return 0;
									}
									
									sc->setType(GT_LTE);
									sc->setRightValue(ast->right->value);
									sc->setRightInfinity(false);
									break;

								case GT:
									
									break;

								case GTE:
									
									break;
							}
							break;

						case GTE:
							out = new SimpleCriteria(GTE_LT, ast->left->variable, ast->right->value, 0, false, true);
							break;
					}
					return out;
				}
			}
			
			out = new CompositeCriteria();
			((CompositeCriteria*)out)->setOperator(ast->type);
			left = buildCriteria(ast->left);
			((CompositeCriteria*)out)->add(left);
			right = buildCriteria(ast->right);
			((CompositeCriteria*)out)->add(right);

			break;
			
		case OR:
			out = new CompositeCriteria();
			((CompositeCriteria*)out)->setOperator(ast->type);
			left = buildCriteria(ast->left);
			((CompositeCriteria*)out)->add(left);
			right = buildCriteria(ast->right);
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