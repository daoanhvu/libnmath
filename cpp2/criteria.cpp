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
#ifdef _WIN32
istream& operator >>(istream& is, const Criteria& c) {
	return is;
}

ostream& nmath::operator <<(ostream& os, const Criteria& c) {
	SimpleCriteria* sc;
	CompositeCriteria *cc;
	int i;
	char opCh = '&';

	switch (c.getCClassType()) {
	case SIMPLE:
		sc = (SimpleCriteria*)(&c);
		switch (sc->getType()) {
		case GT_LT:
			if(!sc->isLeftInfinity())
				os << sc->getLeftValue() << " < ";
			os << sc->getVariable();

			if(!sc->isRightInfinity())
				os << " < " << sc->getRightValue();
			break;

		case GTE_LT:
			os << sc->getLeftValue() << " <= " << sc->getVariable() << " < " << sc->getRightValue();
			break;

		case GT_LTE:
			os << sc->getLeftValue() << " < " << sc->getVariable() << " <= " << sc->getRightValue();
			break;

		case GTE_LTE:
			os << sc->getLeftValue() << " <= " << sc->getVariable() << " <= " << sc->getRightValue();
			break;
		}
		break;

	case COMPOSITE:
		cc = (CompositeCriteria*)(&c);
		os << "{";
		if(cc->logicOperator() == OR) {
			opCh = '|';
		}
		for(i=0; i< cc->size(); i++) {
			os << " " << (*(cc->get(i)));

			if(i<(cc->size()-1))
				os << " " << opCh;
		}
		os << "}";
		break;
	}
	return os;
}
#endif

Criteria* nmath::buildCriteria(const NMAST *ast) {
	Criteria *out = 0;
	Criteria *left, *right;
	SimpleCriteria *sc;
	//double v1, v2;
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
										delete out;
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
			
			left = nmath::buildCriteria(ast->left);
			right = nmath::buildCriteria(ast->right);

			if( (left->getCClassType() == COMPOSITE && right->getCClassType() == SIMPLE) || 
				(right->getCClassType() == COMPOSITE && left->getCClassType() == SIMPLE )) {
				//Here I suspose the left will be the composition,
				//if the left is not composite then we swap left and right
				if(left->getCClassType() != COMPOSITE) {
					Criteria * temp = left;
					left = right;
					right = temp;
				}
				CompositeCriteria * cc = (CompositeCriteria*)left;

				cc->andSelf((SimpleCriteria&)(*right));
				delete right;

				return left;
			}

			out = new CompositeCriteria();
			((CompositeCriteria*)out)->setOperator(ast->type);
			((CompositeCriteria*)out)->add(left);
			((CompositeCriteria*)out)->add(right);

			break;
			
		case OR:
			out = new CompositeCriteria();
			((CompositeCriteria*)out)->setOperator(ast->type);
			left = nmath::buildCriteria(ast->left);
			((CompositeCriteria*)out)->add(left);
			right = nmath::buildCriteria(ast->right);
			((CompositeCriteria*)out)->add(right);
			break;

		case LT:
			out = new SimpleCriteria(GT_LT, ast->variable, 0, ast->value, true, false );
			break;

		case LTE:
			out = new SimpleCriteria(GT_LTE, ast->variable, 0, ast->value, true, false);
			break;

		case GT:
			out = new SimpleCriteria(GT_LT, ast->variable, ast->value, 0, false, true);
			break;

		case GTE:
			out = new SimpleCriteria(GTE_LT, ast->variable, ast->value, 0, false, true);
			break;

		case GT_LT:
		case GT_LTE:
		case GTE_LT:
		case GTE_LTE:
			out = new SimpleCriteria(ast->type, ast->variable, ast->left->value, ast->right->value, false, false);
			break;
	}

	return out;
}