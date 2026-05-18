#include <cstdlib>
#include "SimpleCriteria.h"
#include "compositecriteria.h"

#ifdef _TARGET_HOST_ANDROID
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NativeFunction"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

using namespace nmath;

template <typename T>
SimpleCriteria<T>::SimpleCriteria(): Criteria<T>(SIMPLE) {
	this->rightInfinity = true;
	this->leftInfinity = true;
	this->available = true;
	this->variable = "";
	this->leftVal = 0;
	this->rightVal = 0;
}

template <typename T>
SimpleCriteria<T>::SimpleCriteria(int type, std::string var, T lval, T rval,
										bool leftInfinity, bool rightInfinity): Criteria<T>(SIMPLE) {
	this->type = type;
	this->variable = var;
	this->leftVal = lval;
	this->rightVal = rval;
	this->rightInfinity = rightInfinity;
	this->leftInfinity = leftInfinity;
	this->available = 1;
}

template <typename T>
void SimpleCriteria<T>::copyFrom(SimpleCriteria<T>& c) {
	this->type = c.getType();
	this->variable = c.getVariable();
	this->leftVal = c.getLeftValue();
	this->rightVal = c.getRightValue();
	this->rightInfinity = c.isRightInfinity();
	this->leftInfinity = c.isRightInfinity();
	this->available = c.isAvailable();
}

template <typename T>
Criteria<T>* SimpleCriteria<T>::clone() {
	SimpleCriteria<T> *out;
	out = new SimpleCriteria(type, variable, leftVal, rightVal, leftInfinity, rightInfinity);
	return out;
}

template <typename T>
bool SimpleCriteria<T>::isOverlapped(const SimpleCriteria<T> &c) {
	if (variable == c.variable){
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

/*
Criteria* SimpleCriteria::getIntervalF(const float *values, const char* var, int varCount) {
	Criteria* out;
	int i;
	for (i = 0; i<varCount; i++){
		if (variable == var[i]) {
			out = and(values + (i * 2));
			return out;
		}
	}
	return NULL;
}

Criteria* SimpleCriteria::getInterval(const double *values, const char* var, int varCount) {
	Criteria* out;
	int i;
	for (i = 0; i<varCount; i++){
		if (variable == var[i]) {
			out = and(values + (i * 2));
			return out;
		}
	}

	return NULL;
}
*/
template <typename T>
bool SimpleCriteria<T>::check(const T *values) {
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
	}else if ( this->rightInfinity ) {
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
	}else{
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

template <typename T>
Criteria<T>* SimpleCriteria<T>::andSelf(Criteria<T> &c) {

	if (c.getCClassType() == SIMPLE)
		return andSimpleSelf((SimpleCriteria<T>&)c);

	return andCompositeSelf((CompositeCriteria<T>&)c);
}

template <typename T>
Criteria<T>* SimpleCriteria<T>::andCompositeSelf(CompositeCriteria<T> &c) {
	int i;
	Criteria<T> *out, *outItem;
	Criteria<T> *itm;

	if (c.logicOperator() == AND) {
		// SIMPLE & AND  = out(clone c), add clone SIMPLE to out
		out = c.clone();
		((CompositeCriteria<T>*)out)->add(this);
	}
	else {
		// SIMPLE & OR = SIMPLE & every sub OR
		out = new CompositeCriteria<T>();
		((CompositeCriteria<T>*)out)->setOperator(OR);

		for (i = 0; i<c.size(); i++) {
			itm = c[i];
			outItem = nullptr;
			switch (itm->getCClassType()) {
			case SIMPLE:
				outItem = this->andSimpleSelf((SimpleCriteria<T>&)*itm);
				break;

			case COMPOSITE:
				outItem = this->andCompositeSelf((CompositeCriteria<T>&)*itm);
				break;
			}

			if (outItem != nullptr)
				((CompositeCriteria<T>*)out)->add(outItem);
		}
	}
	return out;
}

template <typename T>
Criteria<T>* SimpleCriteria<T>::andSimpleSelf(SimpleCriteria<T> &c) {
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

template <typename T>
Criteria<T>* SimpleCriteria<T>::andCriteria(SimpleCriteria<T> &c) {
	Criteria<T>* out = nullptr;
	T d[2] = {c.leftVal, c.rightVal};

	if(this->variable == c.variable) {	
		if( !(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
				out = this->andValue(d);
			} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
				if(this->leftVal >= c.leftVal) {
					/*
						This   |--------|
						c    |-----------
					*/
					out = this->clone();
				} else if( (this->leftVal < c.leftVal) && (this->rightVal >= c.leftVal) ) {
					/*
						This   |--------|
						c        |---------
					*/
					out = this->clone();
					((SimpleCriteria<T>*)out)->leftVal = c.leftVal;
				}
			} else if( (c.leftInfinity) && !(c.rightInfinity) ) { //c is close on right and open on left 110
				if( this->rightVal <= c.rightVal) {
					/*
						This   |--------|
						c     ---------------|
					*/
					out = this->clone();
				} else if( (c.rightVal<=this->rightVal) && (c.rightVal > this->leftVal) )  {
					/*
						This   |--------|
						c     --------|
					*/
					out = this->clone();
					((SimpleCriteria<T>*)out)->rightVal = c.rightVal;
				}
			}
		} if( !(this->leftInfinity) && (this->rightInfinity) ) { /* This interval is close on LEFT and open on RIGHT  */
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
				if(c.leftVal >= this->leftVal) {
					/*
						This   |-----------------
						c    	|-----------|
					*/
					out = c.clone();
				} else if(c.leftVal < this->leftVal) {
					/*
						This   |-----------------
						c    |-----------|
					*/
					out = c.clone();
					((SimpleCriteria<T>*)out)->leftVal = this->leftVal;
				}
			} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
				if( c.leftVal <= this->leftVal ) {
					/*
						This   |----------
						c    |-----------
					*/
					out = this->clone();
				} else {
					/*
						This   |--------
						c        |---------
					*/
					out = c.clone();
				}
			} else if( (c.leftInfinity) && !(c.rightInfinity) ) { //c is close on right and open on left 110
				if( this->leftVal <= c.rightVal ) {
					/*
						This   |-------------
						c     ---------------|
					*/
					out = c.clone();
					((SimpleCriteria<T>*)out)->leftVal = this->leftVal;
				} else {
					/*
						This   			|--------
						c     --------|
					*/
					out = nullptr;
				}
			} else { // c is open
				out = this->clone();
			}
		} if( (this->leftInfinity) && !(this->rightInfinity) ) { /* This interval is close on RIGHT and open on LEFT  */
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
				if( c.rightVal <= this->rightVal) {
					/*
						This   -----------------|
						c    	|-----------|
					*/
					out = c.clone();
				} else {
					if(c.leftVal <= this->rightVal) {
						/*
							This   ---------|
							c    |------------|
						*/
						out = c.clone();
						((SimpleCriteria<T>*)out)->rightVal = this->rightVal;
					} else {
						/*
							This   ----|
							c    		 |------|
						*/
						out = nullptr;
					}
				}
			} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
				if( c.leftVal <= this->rightVal ) {
					/*
						This   --------|
						c    |-----------
					*/
					out = c.clone();
					((SimpleCriteria<T>*)out)->rightVal = this->rightVal;
				} else {
					/*
						This   ---|
						c        	|-----
					*/
					out = nullptr;
				}
			} else if( (c.leftInfinity) && !(c.rightInfinity) ) { //c is close on right and open on left 110
				if( c.rightVal >= this->rightVal ) {
					/*
						This   --------|
						c     ------------|
					*/
					out = this->clone();
				} else {
					/*
						This   --------|
						c     ------|
					*/
					out = c.clone();
				}
			} else {		//c is OPEN
				out = this->clone();
			}
		} else { //this is OPEN
			out = c.clone();
		}
	} else {
		out = new CompositeCriteria<T>();
		((CompositeCriteria<T>*)out)->setOperator(AND);
		((CompositeCriteria<T>*)out)->add(this->clone());
		((CompositeCriteria<T>*)out)->add(c.clone());
	}

	return out;
}

template <typename T>
Criteria<T>* SimpleCriteria<T>::andCriteria(CompositeCriteria<T> &c) {
	int i;
	Criteria<T> *out, *outItem;
	Criteria<T> *itm;

	if(c.logicOperator() == AND) {
		// SIMPLE & AND  = out(clone c), add clone SIMPLE to out
		out = c.clone();
		((CompositeCriteria<T>*)out)->add(this->clone());
	} else {
		// SIMPLE & OR = SIMPLE & every sub OR
		out = new CompositeCriteria<T>();
		((CompositeCriteria<T>*)out)->setOperator(OR);
		
		for(i=0; i<c.size(); i++) {
			itm = c[i];
			outItem = 0;
			switch(itm->getCClassType()) {
				case SIMPLE:
					outItem = this->andCriteria((SimpleCriteria<T>&)*itm);
				break;
				
				case COMPOSITE:
					outItem = this->andCriteria((CompositeCriteria<T>&)*itm);
				break;
			}
			if(outItem != 0)
				((CompositeCriteria<T>*)out)->add(outItem);
		}
	}
	return out;
}

template <typename T>
Criteria<T>* SimpleCriteria<T>::operator &(Criteria<T> &c) {
	switch (c.getCClassType()){
		case SIMPLE:
			return andCriteria((SimpleCriteria<T>&)c);

		case COMPOSITE:
			return andCriteria((CompositeCriteria<T>&)c);
        default: return nullptr;
	}
}

template <typename T>
Criteria<T>* SimpleCriteria<T>::orCriteria(SimpleCriteria<T>& c) {
	Criteria<T>* out = nullptr;
	bool usedComposite = false;

	if(this->variable == c.variable) { /* This criteria and c have the same variable */
		if( !(this->leftInfinity) && !(this->rightInfinity) ) { //If this interval is close
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is close
				// process exceptions first
				if( (this->rightVal < c.leftVal) || (this->leftVal > c.rightVal) ){
					/*
						this  |---|
						c  			 |-----|
					OR
						this        |----|
						c    |----|
					*/
					usedComposite = true;
				} else {
					if(this->leftVal <= c.leftVal) {
						out = this->clone();
						if(this->rightVal < c.rightVal) {
							/*
							Default: this  |--------|
									 c    	|----|
							OR
							this  |--------|
							c    	|-------|
							*/
							((SimpleCriteria<T>*)out)->rightVal = c.rightVal;
						}
					} else {
						/*
							Default: this    |--------|
									 c    	|----------|
						*/
						out = c.clone();
						if(this->rightVal < c.rightVal) {
							/*
							this      |--------|
							c    	|-------|
							*/
							((SimpleCriteria<T>*)out)->rightVal = this->rightVal;
						}
					}
				}
				
			} else if( !(c.leftInfinity) && c.rightInfinity ) { // c is close on LEFT and open on RIGHT
				//exception first
				if(this->rightVal < c.leftVal ) {
					/*
						this  |---|
						c  			|-----
					*/
					usedComposite = true;
				} else {
					/*
						default	this       |--------|
								c    	|-------------
						*/
					out = c.clone();
					if(this->leftVal <= c.leftVal) {
						/*
							this  |--------|
							c    	|------------
						*/
						
						((SimpleCriteria<T>*)out)->leftVal = this->leftVal;
					}
				}
			} else if( c.leftInfinity && !(c.rightInfinity) ) { //c is open on LEFT and close on RIGHT
				//exception first
				if( this->leftVal > c.rightVal ) {
					/*
						this 			|-----|
						c 		-----|
					*/
					usedComposite = true;
				} else {

				}

			} else { // c is open
				out = c.clone();
			}
		} if( !(this->leftInfinity) && this->rightInfinity ) { /* This interval is close on RIGHT and open on LEFT  */

		} if( this->leftInfinity && !(this->rightInfinity) ) { /* This interval is close on LEFT and open on RIGHT  */

		} else { // this object is open
			out = this->clone();
		}
	} else {
	    usedComposite = true;
	}


	if(usedComposite) {
		out = new CompositeCriteria<T>();
		((CompositeCriteria<T>*)out)->setOperator(OR);
		((CompositeCriteria<T>*)out)->add(this->clone());
		((CompositeCriteria<T>*)out)->add(c.clone());
	}
	
	return out;
}

template <typename T>
CompositeCriteria<T>* SimpleCriteria<T>::orCriteria(CompositeCriteria<T> &c) {
	auto *out = (CompositeCriteria<T>*)c.clone();
	out->add(clone());

	return out;
}

/**
	Need to implement
*/
template <typename T>
Criteria<T>* SimpleCriteria<T>::operator |(Criteria<T> &c) {
	switch (c.getCClassType()) {
		case SIMPLE:
			return orCriteria((SimpleCriteria<T>&)c);
		case COMPOSITE:
			return orCriteria((CompositeCriteria<T>&)c);
		default:
			return nullptr;
	}
}

template <typename T>
SimpleCriteria<T>& SimpleCriteria<T>::operator=(const nmath::SimpleCriteria<T> &c) {
    this->type = c.getType();
    this->variable = c.getVariable();
    this->leftVal = c.getLeftValue();
    this->rightVal = c.getRightValue();
    this->rightInfinity = c.isRightInfinity();
    this->leftInfinity = c.isRightInfinity();
    this->available = c.isAvailable();
}

