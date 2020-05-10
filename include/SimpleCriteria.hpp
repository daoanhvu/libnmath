#ifndef __SIMPLECRITERIA_H_
#define __SIMPLECRITERIA_H_

//#include "common.h"
#include <string>
#include "criteria.hpp"
#include "compositecriteria.hpp"

namespace nmath {

    template <typename T>
	class SimpleCriteria: public Criteria<T> {
		private:
			/** GT_LT, GTE_LT, GT_LTE, GTE_LTE */
			int type;
			std::string variable;
			T leftVal;
			T rightVal;
			bool rightInfinity;
			bool leftInfinity;

			Criteria<T>* andSimpleSelf(SimpleCriteria<T> &c) {
				T d[2] = {c.leftVal, c.rightVal};
				Criteria<T>* out = nullptr;
				SimpleCriteria<T> *tmp;
				
				if(this->variable == c.variable) {	
					if( !(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
						if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
							tmp = this->andValue(d);
							if(tmp != nullptr) {
								copyFrom(*tmp);
								delete tmp;
							}
							
						} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
							/* For this case: return this object
								This   |--------|
								c    |-----------
							*/
								
							if( (this->leftVal < c.leftVal) && (this->rightVal >= c.leftVal) ) {
								/*
									This   |--------|
									c        |---------
								*/
								
								this->leftVal = c.leftVal;
							}
						} else if( (c.leftInfinity) && !(c.rightInfinity) ) { //c is close on right and open on left 110
							/*
								This   |--------|
								c     ---------------|
							*/
							
							if( (c.rightVal<=this->rightVal) && (c.rightVal > this->leftVal) )  {
								/*
									This   |--------|
									c     --------|
								*/
								this->rightVal = c.rightVal;
							}
						}
					} if( !(this->leftInfinity) && (this->rightInfinity) ) { /* This interval is close on LEFT and open on RIGHT  */
						if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
							if(c.leftVal >= this->leftVal) {
								/*
									This   |-----------------
									c    	|-----------|
								*/
								copyFrom(c);
							} else if(c.leftVal < this->leftVal) {
								/*
									This   |-----------------
									c    |-----------|
								*/
								this->type = c.getType();
								this->variable = c.getVariable();
								//Dont copy leftValue
								this->rightVal = c.getRightValue();
								this->rightInfinity = c.isRightInfinity();
								this->leftInfinity = c.isRightInfinity();
								this->available = c.isAvailable();
							}
						} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
							/*
								This   |----------
								c    |-----------
							*/
							
							/*
								This   |----------
								c    ----------------
							*/
							
							if( c.leftVal > this->leftVal ) {
								/*
									This   |--------
									c        |---------
								*/
								copyFrom(c);
							}
						} else if( (c.leftInfinity) && !(c.rightInfinity) ) { //c is close on right and open on left 110
							if( this->leftVal <= c.rightVal ) {
								/*
									This   |-------------
									c     ---------------|
								*/
								this->rightVal = c.getRightValue();
							} else {
								/*
									This   			|--------
									c     --------|
								*/
								//out = NULL;
								return nullptr;
							}
						}
					} if( (this->leftInfinity) && !(this->rightInfinity) ) { /* This interval is close on RIGHT and open on LEFT  */
						if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
							if( c.rightVal <= this->rightVal) {
								/*
									This   -----------------|
									c    	|-----------|
								*/
								copyFrom(c);
							} else {
								if(c.leftVal <= this->rightVal) {
									/*
										This   ---------|
										c    |------------|
									*/
									this->leftVal = c.getLeftValue();
								} else {
									/*
										This   ----|
										c    		 |------|
									*/
									//out = NULL;
									return nullptr;
								}
							}
						} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
							if( c.leftVal <= this->rightVal ) {
								/*
									This   --------|
									c    |-----------
								*/
								this->leftVal = c.getLeftValue();
							} else {
								/*
									This   ---|
									c        	|-----
								*/
								//out = NULL;
								return nullptr;
							}
						} else if( (c.leftInfinity) && !(c.rightInfinity) ) { //c is close on right and open on left 110
							/*
								This   --------|
								c     ------------|
							*/
							
							if( c.rightVal < this->rightVal ) {
								/*
									This   --------|
									c     ------|
								*/
								copyFrom(c);
							}
						}
					} else { //this is OPEN
						copyFrom(c);
					}
					
					out = this;
				} else {
					out = new CompositeCriteria<T>();
					((CompositeCriteria<T>*)out)->setOperator(AND);
					((CompositeCriteria<T>*)out)->add(this);
					((CompositeCriteria<T>*)out)->add(c.clone());
				}

				return out;
			}
			// Criteria<T>* andCompositeSelf(CompositeCriteria<T> &c);

		public:
			SimpleCriteria<T>(): Criteria<T>(NMathCClassType::SIMPLE) {
				this->rightInfinity = true;
				this->leftInfinity = true;
				this->available = true;
				this->variable = "";
				this->leftVal = 0;
				this->rightVal = 0;
			}

			SimpleCriteria<T>(int type, std::string var, T lval, T rval,
											bool leftInfinity, bool rightInfinity): Criteria<T>(NMathCClassType::SIMPLE) {
				this->type = type;
				this->variable = var;
				this->leftVal = lval;
				this->rightVal = rval;
				this->rightInfinity = rightInfinity;
				this->leftInfinity = leftInfinity;
				this->available = true;
			}

			int getType() const	{ return type; }
			void setType(int t) { type = t; }

			std::string getVariable() const { return variable; }
			void setVariable(const char *var) { variable = var; }

			T getLeftValue() const override { return leftVal; }
			void setLeftValue(T v) { leftVal = v; }
			T getRightValue() const override { return rightVal; }
			void setRightValue(T v) { rightVal = v; }
			bool isRightInfinity() const { return rightInfinity; }
			void setRightInfinity(bool rInf) { rightInfinity = rInf; }
			bool isLeftInfinity() const { return leftInfinity; }
			void setLeftInfinity(bool lInf) { leftInfinity = lInf; }
			Criteria<T>& normalize(std::vector<nmath::NMAST<T>* > variables) override {
				return *this;
			}

			bool containsVar(const std::string &var) override { return variable == var; }
			
			bool isOverlapped(const SimpleCriteria<T> &c) {
				if (variable == c.variable) {
					if (!(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
						if (!(c.leftInfinity) && !(c.rightInfinity)) { //c is closed

			                /*
			                    this |---|
			                    c			|---|
			                    OR
			                    this 		|---|
			                    c	  |---|
			                    */
			                return !((rightVal < c.leftVal) || (leftVal > c.rightVal));
						} else if (!(c.leftInfinity) && c.rightInfinity) { // c close on LEFT, open on RIGHT

			                /*
			                    this |--|
			                    c		 |---
			                    */
			                return rightVal >= c.leftVal;
						} else if (c.leftInfinity && !c.rightInfinity) { // c OPEN on LEFT, CLOSE on RIGHT
			                /*
			                        this      |-----|
			                        c     ---|
			                    */
			                return this->leftVal <= c.rightVal;
						}
					}
				}

				return false;
			}

			/**
			 *  Number of element in values is double in variables
			*/
//			template <typename T>
			Criteria<T>* getInterval(const T *values, const std::vector<std::string> &variables) override {
				Criteria<T> *out;
				for (int i = 0; i<variables.size(); i++){
					if (variable == variables[i]) {
						out = andValue(values + (i * 2));
						return out;
					}
				}

				return nullptr;
			}

			Criteria<T>* andSelf(Criteria<T> &c) override {
				if(c.getCClassType() == SIMPLE ) {
					return andSimpleSelf((SimpleCriteria<T>&)c);
				}
				return andCompositeSelf((CompositeCriteria<T>&)c);
			}

			Criteria<T>* andCriteria(Criteria<T> &c) {
				Criteria<T> *out;
				switch (c.getCClassType()){
					case SIMPLE:
						out = andSimpleSelf((SimpleCriteria<T>&)c);
						break;

					case COMPOSITE:
						out = andCompositeSelf((CompositeCriteria<T>&)c);
						break;
				}
				return out;
			}

			Criteria<T>* andCompositeSelf(CompositeCriteria<T> &c) {
				//TODO: To be implemented
				return NULL;
			}

			Criteria<T>* orCriteria(Criteria<T> &c) {
				//TODO: To be implemented
				return NULL;
			}

			//CompositeCriteria<T>* orCriteria(CompositeCriteria<T> &c);

			/* And this criteria with pair of values */
			SimpleCriteria<T>* andValue(const T *values) {
				SimpleCriteria<T> *outInterval = nullptr;

				if (this->leftInfinity && this->rightInfinity) {
					outInterval = new SimpleCriteria<T>();
                    outInterval->variable = variable;
					outInterval->available = true;
					outInterval->leftInfinity = false;
					outInterval->rightInfinity = false;
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
						// mX < rightVal
						if (this->rightVal <= values[0]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}

						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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
						// mX <= rightVal
						if (this->rightVal < values[0]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}

						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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
						// leftVal < mX
						if (this->leftVal >= values[1]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}
						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
						outInterval->rightVal = values[1];
						outInterval->type = GT_LTE; //TODO: need to test here
						if (this->leftVal < values[0])
							outInterval->leftVal = values[0];
						else
							outInterval->leftVal = this->leftVal;
						break;

					case GTE_LT:
					case GTE_LTE:
						// leftVal <= mX
						if (this->leftVal > values[1]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}
						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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
						// leftVal < mX < rightVal
						if (this->leftVal >= values[1] || this->rightVal <= values[0]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}

						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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
							return nullptr;
						}
						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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
						// leftVal <= mX < rightVal
						if (this->leftVal > values[1] || this->rightVal <= values[0]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}
						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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
						// leftVal <= mX <= rightVal
						if (this->leftVal > values[1] || this->rightVal < values[0]){
							//return empty set, available bit set to FALSE
							return nullptr;
						}
						outInterval = new SimpleCriteria<T>();
						outInterval->variable = variable;
						outInterval->available = true;
						outInterval->leftInfinity = false;
						outInterval->rightInfinity = false;
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

			Criteria<T>* operator &(Criteria<T>& c) override {
				return andCriteria(c);
			}
			Criteria<T>* operator |(Criteria<T>& c) override {
				return orCriteria(c);
			}

			Criteria<T>* clone() override {
				SimpleCriteria<T> *out = new SimpleCriteria<T>(type, variable, leftVal, rightVal, leftInfinity, rightInfinity);
				return out;
			}

			void copyFrom(SimpleCriteria<T>& c) {
				this->type = c.getType();
				this->variable = c.getVariable();
				this->leftVal = c.getLeftValue();
				this->rightVal = c.getRightValue();
				this->rightInfinity = c.isRightInfinity();
				this->leftInfinity = c.isRightInfinity();
				this->available = c.isAvailable();
			}

			SimpleCriteria<T>& operator=(const SimpleCriteria<T> &c) {
				this->type = c.getType();
			    this->variable = c.getVariable();
			    this->leftVal = c.getLeftValue();
			    this->rightVal = c.getRightValue();
			    this->rightInfinity = c.isRightInfinity();
			    this->leftInfinity = c.isRightInfinity();
			    this->available = c.isAvailable();
			}

			/* check if value does belong to this interval */
			bool check(const T* values) override {
				bool result = false;
				
				if( (this->leftInfinity) && (this->rightInfinity) )
					return true;
					
				if( this->leftInfinity ) {
					/** HERE we don't need to take care of leftVal */
					switch(this->type){
						case GT_LT:
						case GTE_LT:
							// mX < rightVal
							if((*values) < this->rightVal)
								result = true;
						break;
						
						case GT_LTE:
						case GTE_LTE:
							// mX <= rightVal
							if ((*values) <= this->rightVal)
								result = true;
						break;
					}
				} else if ( this->rightInfinity ) {
					/** HERE we don't need to take care of rightVal */
					switch(this->type){
						case GT_LT:
						case GT_LTE:
							// leftVal < mX
							if (this->leftVal < (*values))
								result = true;
						break;
						
						case GTE_LT:
						case GTE_LTE:
							// leftVal <= mX
							if (this->leftVal <= (*values))
								result = true;
						break;
					}
				} else {
					switch(this->type){
						case GT_LT:
							// leftVal < mX < rightVal
							if ((this->leftVal < (*values)) && ((*values) < this->rightVal))
								result = true;
						break;
						
						case GT_LTE:
							// leftVal < mX <= rightVal
							if ((this->leftVal < (*values)) && ((*values) <= this->rightVal))
								result = true;
						break;
						
						case GTE_LT:
							// leftVal <= mX < rightVal
							if ((this->leftVal <= (*values)) && ((*values) < this->rightVal))
								result = true;
						break;
						
						case GTE_LTE:
							// leftVal <= mX <= rightVal
							if ((this->leftVal <= (*values)) && ((*values) <= this->rightVal))
								result = true;
						break;
					}
				}
				
				return result;
			}
	};
}

#endif
