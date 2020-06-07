#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#ifdef _WIN32
#include <iostream>
#endif

#include "common.hpp"

// #include "SimpleCriteria.hpp"
//#include "compositecriteria.hpp"

namespace nmath {
	template <typename T> class SimpleCriteria;
	template <typename T> class CompositeCriteria;

	enum NMathCClassType {
		SIMPLE, COMPOSITE
	};

	template <typename T>
	class Criteria {
		private:

		protected:
			bool available;
			NMathCClassType cType;

		public:
			Criteria(NMathCClassType type) { this->cType = type; }
			virtual ~Criteria(){ }
			bool isAvailable() const { return available; }
			NMathCClassType getCClassType() const { return cType; }

			virtual Criteria<T>* andSelf(Criteria<T> &c) { return nullptr; }

			virtual Criteria<T>* operator &(Criteria<T> &c) { return this; }
			virtual Criteria<T>* operator |(Criteria<T> &c) { return this; }
#ifdef _PCDEBUG
			friend std::istream& operator >>(std::istream& is, const Criteria& c);
			friend std::ostream& operator <<(std::ostream& os, const Criteria& c);
#endif
			virtual T getLeftValue() const = 0;
			virtual T getRightValue() const = 0;
			/* check if value does belong to this interval */
			virtual bool check(const T *value) { return false; }
			virtual Criteria<T>* clone() { return nullptr; }
			virtual bool containsVar(const std::string &var) { return false; }
			virtual Criteria<T>* getInterval(const T *values, const std::vector<std::string> &variables) { return nullptr; }
			virtual Criteria<T>& normalize(std::vector<NMAST<T>* > variables) = 0;
	};

#ifdef _PCDEBUG
	std::istream& operator >>(std::istream& is, const Criteria& c);
	std::ostream& operator <<(std::ostream& os, const Criteria& c);
#endif

	template <typename T>
	Criteria<T>* buildCriteria(const NMAST<T> *ast) {
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
}


#endif
