#ifndef __SIMPLECRITERIA_H_
#define __SIMPLECRITERIA_H_

//#include "common.h"
#include "criteria.h"

namespace nmath {

	class CompositeCriteria;

	class SimpleCriteria: public Criteria {
		private:
			/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
			int type;
			char variable;
			double leftVal;
			double rightVal;
			bool rightInfinity;
			bool leftInfinity;

			Criteria* andSimpleSelf(SimpleCriteria& c);
			Criteria* andCompositeSelf(CompositeCriteria& c);

		public:
			SimpleCriteria();
			SimpleCriteria(int type, char var, double lval, double rval, 
											bool leftInfinity, bool rightInfinity);

			int getType() const	{ return type; }
			void setType(int t) { type = t; }

			char getVariable() const { return variable; }
			void setVariable(char var) { variable = var; }

			double getLeftValue() const { return leftVal; }
			void setLeftValue(double v) { leftVal = v; }
			double getRightValue() const { return rightVal; }
			void setRightValue(double v) { rightVal = v; }
			bool isRightInfinity() const { return rightInfinity; }
			void setRightInfinity(bool rInf) { rightInfinity = rInf; }
			bool isLeftInfinity() const { return leftInfinity; }
			void setLeftInfinity(bool lInf) { leftInfinity = lInf; }
			

			bool containsVar(char var) { return (variable == var); }
			
			bool isOverlapped(const SimpleCriteria& c);

			template <typename T>
			Criteria* getInterval(const T *values, const char* var, int varCount) {
				Criteria* out;
				int i;
				for (i = 0; i<varCount; i++){
					if (variable == var[i]) {
						out = andValue(values + (i * 2));
						return out;
					}
				}

				return NULL;
			}

			Criteria* andSelf(Criteria& c);
			Criteria* andCriteria(SimpleCriteria& c);
			Criteria* andCriteria(CompositeCriteria& c);

			Criteria* orCriteria(SimpleCriteria& c);
			CompositeCriteria* orCriteria(CompositeCriteria& c);

			/* And this criteria with pair of values */
			template <typename VT>
			SimpleCriteria* andValue(const VT *values) {
				SimpleCriteria *outInterval = 0;

				if (this->leftInfinity && this->rightInfinity) {
					outInterval = new SimpleCriteria();
					outInterval->variable = variable;
					outInterval->available = 1;
					outInterval->leftInfinity = 0;
					outInterval->rightInfinity = 0;
					outInterval->leftVal = values[0];
					outInterval->rightVal = values[1];
					outInterval->type = GTE_LTE;
					return outInterval;
				}

				if (this->leftInfinity) {
					/** HERE we don't need to take care of leftVal */
					switch (this->type){
					case GT_LT:
					case GTE_LT:
						// x < rightVal
						if (this->rightVal <= values[0]){
							//return empty set, available bit set to FALSE
							return 0;
						}

						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->leftVal = values[0];
						outInterval->type = GTE_LT; //TODO: need to test here
						if (values[1] < this->rightVal)
							outInterval->rightVal = values[1];
						else
							outInterval->rightVal = this->rightVal;
						//outInterval->rightVal = this->rightVal - epsilon;
						break;

					case GT_LTE:
					case GTE_LTE:
						// x <= rightVal
						if (this->rightVal < values[0]){
							//return empty set, available bit set to FALSE
							return 0;
						}

						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->leftVal = values[0];
						outInterval->type = GTE_LTE; //TODO: need to test here
						if (values[1] <= this->rightVal)
							outInterval->rightVal = values[1];
						else
							outInterval->rightVal = this->rightVal;
						break;
					}
				}
				else if (this->rightInfinity) {
					/** HERE we don't need to take care of rightVal */
					switch (this->type){
					case GT_LT:
					case GT_LTE:
						// leftVal < x
						if (this->leftVal >= values[1]){
							//return empty set, available bit set to FALSE
							return 0;
						}
						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->rightVal = values[1];
						outInterval->type = GT_LTE; //TODO: need to test here
						if (this->leftVal < values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;
						break;

					case GTE_LT:
					case GTE_LTE:
						// leftVal <= x
						if (this->leftVal > values[1]){
							//return empty set, available bit set to FALSE
							return 0;
						}
						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->rightVal = values[1];
						outInterval->type = GTE_LTE; //TODO: need to test here
						if (this->leftVal <= values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;
						break;
					}
				}
				else{
					switch (this->type){
					case GT_LT:
						// leftVal < x < rightVal
						if (this->leftVal >= values[1] || this->rightVal <= values[0]){
							//return empty set, available bit set to FALSE
							return 0;
						}

						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->type = GTE_LTE;

						if (this->leftVal < values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;

						if (values[1] < this->rightVal)
							outInterval->rightVal = values[1];
						else
							outInterval->rightVal = this->rightVal;

						break;

					case GT_LTE:
						if (this->leftVal >= values[1] || this->rightVal < values[0]){
							//return empty set, available bit set to FALSE
							return 0;
						}
						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->type = GTE_LTE;
						if (this->leftVal < values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;

						if (values[1] <= this->rightVal)
							outInterval->rightVal = values[1];
						else
							outInterval->rightVal = this->rightVal;

						break;

					case GTE_LT:
						// leftVal <= x < rightVal
						if (this->leftVal > values[1] || this->rightVal <= values[0]){
							//return empty set, available bit set to FALSE
							return 0;
						}
						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->type = GTE_LTE;
						if (this->leftVal <= values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;

						if (values[1] < this->rightVal)
							outInterval->rightVal = values[1];
						else
							outInterval->rightVal = this->rightVal;
						break;

					case GTE_LTE:
						// leftVal <= x <= rightVal
						if (this->leftVal > values[1] || this->rightVal < values[0]){
							//return empty set, available bit set to FALSE
							return 0;
						}
						outInterval = new SimpleCriteria();
						outInterval->variable = variable;
						outInterval->available = 1;
						outInterval->leftInfinity = 0;
						outInterval->rightInfinity = 0;
						outInterval->type = GTE_LTE;
						if (this->leftVal <= values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;

						if (values[1] <= this->rightVal)
							outInterval->rightVal = values[1];
						else
							outInterval->rightVal = this->rightVal;
						break;
					}
				}

				return outInterval;
			}
#ifdef _WIN32
			istream& operator >>(istream& is);
			ostream& operator <<(ostream& os);
#endif
			Criteria* operator &(Criteria& c);
			Criteria* operator |(Criteria& c);

			Criteria* clone();
			void copyFrom(SimpleCriteria& c);
			SimpleCriteria& operator =(SimpleCriteria& c2);

			/* check if value does belong to this interval */
			bool check(const double* value);
	};
}

#endif