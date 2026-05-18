#include "criteria.h"
#include "SimpleCriteria.h"
#include "compositecriteria.h"

using namespace nmath;

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
#ifdef _PCDEBUG
std::istream& nmath::operator >>(std::istream& is, const Criteria& c) {
	return is;
}

std::ostream& nmath::operator <<(std::ostream& os, const Criteria& c) {
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
			os << " " << ((Criteria&)*(cc->get(i)));

			if(i<(cc->size()-1))
				os << " " << opCh;
		}
		os << "}";
		break;
	}
	return os;
}
#endif

template <typename T>
Criteria<T>* nmath::buildCriteria(const NMAST<T> *ast) {
	Criteria<T> *out = 0;
	Criteria<T> *left, *right;
	SimpleCriteria<T> *sc;
	//double v1, v2;
	switch (ast->type) {
		case AND:
			if(nmath::isComparationOperator(ast->left->type) && nmath::isComparationOperator(ast->right->type)) {
				if( ast->left->text == ast->right->text){
					switch(ast->left->type) {
						case LT:
							out = new SimpleCriteria<T>(GT_LT, ast->left->text, 0, ast->right->value, true, false );
							sc = (SimpleCriteria<T>*)out;
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
							out = new SimpleCriteria<T>(GT_LTE, ast->left->text, 0, ast->right->value, true, false);
							break;

						case GT:
							out = new SimpleCriteria<T>(GT_LT, ast->left->text, ast->left->value, 0, false, true);
							sc = (SimpleCriteria<T>*)out;
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
							out = new SimpleCriteria<T>(GTE_LT, ast->left->text, ast->right->value, 0, false, true);
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
					Criteria<T> *temp = left;
					left = right;
					right = temp;
				}
				CompositeCriteria<T> * cc = (CompositeCriteria<T>*)left;

				cc->andSelf((SimpleCriteria<T>&)(*right));
				delete right;

				return left;
			}

			out = new CompositeCriteria<T>();
			((CompositeCriteria<T>*)out)->setOperator(ast->type);
			((CompositeCriteria<T>*)out)->add(left);
			((CompositeCriteria<T>*)out)->add(right);

			break;
			
		case OR:
			out = new CompositeCriteria<T>();
			((CompositeCriteria<T>*)out)->setOperator(ast->type);
			left = nmath::buildCriteria(ast->left);
			((CompositeCriteria<T>*)out)->add(left);
			right = nmath::buildCriteria(ast->right);
			((CompositeCriteria<T>*)out)->add(right);
			break;

		case LT:
			out = new SimpleCriteria<T>(GT_LT, ast->text, 0, ast->value, true, false );
			break;

		case LTE:
			out = new SimpleCriteria<T>(GT_LTE, ast->text, 0, ast->value, true, false);
			break;

		case GT:
			out = new SimpleCriteria<T>(GT_LT, ast->text, ast->value, 0, false, true);
			break;

		case GTE:
			out = new SimpleCriteria<T>(GTE_LT, ast->text, ast->value, 0, false, true);
			break;

		case GT_LT:
		case GT_LTE:
		case GTE_LT:
		case GTE_LTE:
			out = new SimpleCriteria<T>(ast->type, ast->text, ast->left->value, ast->right->value, false, false);
			break;
	}

	return out;
}
