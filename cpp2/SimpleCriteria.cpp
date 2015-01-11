#include <stdlib.h>
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

SimpleCriteria::SimpleCriteria() {
	this->cType = SIMPLE;
	this->rightInfinity = 1;
	this->leftInfinity = 1;
	this->available = 1;
	this->type = type;
	this->variable = 0;
	this->leftVal = 0;
	this->rightVal = 0;
}

SimpleCriteria::SimpleCriteria(int type, char var, double lval, double rval,
										bool leftInfinity, bool rightInfinity) {
	this->cType = SIMPLE;
	this->type = type;
	this->variable = var;
	this->leftVal = lval;
	this->rightVal = rval;
	this->rightInfinity = rightInfinity;
	this->leftInfinity = leftInfinity;
	this->available = 1;
}

void SimpleCriteria::copyFrom(SimpleCriteria& c) {
	this->type = c.getType();
	this->variable = c.getVariable();
	this->leftVal = c.getLeftValue();
	this->rightVal = c.getRightValue();
	this->rightInfinity = c.isRightInfinity();
	this->leftInfinity = c.isRightInfinity();
	this->available = c.isAvailable();
}

Criteria* SimpleCriteria::clone() {
	SimpleCriteria *out;
	out = new SimpleCriteria(type, variable, leftVal, rightVal, leftInfinity, rightInfinity);
	return out;
}

bool SimpleCriteria::isOverlapped(const SimpleCriteria& c) {
	if (variable == c.variable){
		if (!(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
			if (!(c.leftInfinity) && !(c.rightInfinity)) { //c is closed
				if ((rightVal < c.leftVal) || (leftVal > c.rightVal)){
					/*
					this |---|
					c			|---|
					OR
					this 		|---|
					c	  |---|
				
					*/
					return false;
				} else {
					return true;
				}
			} else if (!(c.leftInfinity) && c.rightInfinity) { // c close on LEFT, open on RIGHT
				if(rightVal < c.leftVal){
					/*
					this |--|
					c		 |---
					*/
					return false;
				} else return true;
			} else if (c.leftInfinity && !c.rightInfinity) { // c OPEN on LEFT, CLOSE on RIGHT
				if(this->leftVal > c.rightVal) {
					/*
						this      |-----|
						c     ---|
					*/
					return false;
				} else
					return true;
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
bool SimpleCriteria::check(const double* values) {
	bool result = false;
	
	if( (this->leftInfinity) && (this->rightInfinity) )
		return true;
		
	if( this->leftInfinity ) {
		/** HERE we don't need to take care of leftVal */
		switch(this->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if((*values) < this->rightVal)
					result = true;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if ((*values) <= this->rightVal)
					result = true;
			break;
		}
	}else if ( this->rightInfinity ) {
		/** HERE we don't need to take care of rightVal */
		switch(this->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if (this->leftVal < (*values))
					result = true;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if (this->leftVal <= (*values))
					result = true;
			break;
		}
	}else{
		switch(this->type){
			case GT_LT:
				// leftVal < x < rightVal
				if ((this->leftVal < (*values)) && ((*values) < this->rightVal))
					result = true;
			break;
			
			case GT_LTE:
				// leftVal < x <= rightVal
				if ((this->leftVal < (*values)) && ((*values) <= this->rightVal))
					result = true;
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if ((this->leftVal <= (*values)) && ((*values) < this->rightVal))
					result = true;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if ((this->leftVal <= (*values)) && ((*values) <= this->rightVal))
					result = true;
			break;
		}
	}
	
	return result;
}

Criteria* SimpleCriteria::andSelf(Criteria& c) {

	if (c.getCClassType() == SIMPLE)
		return andSimpleSelf((SimpleCriteria&)c);

	return andCompositeSelf((CompositeCriteria&)c);
}

Criteria* SimpleCriteria::andCompositeSelf(CompositeCriteria& c) {
	int i;
	Criteria *out, *outItem;
	Criteria *itm;

	if (c.logicOperator() == AND) {
		// SIMPLE & AND  = out(clone c), add clone SIMPLE to out
		out = c.clone();
		((CompositeCriteria*)out)->add(this);
	}
	else {
		// SIMPLE & OR = SIMPLE & every sub OR
		out = new CompositeCriteria();
		((CompositeCriteria*)out)->setOperator(OR);

		for (i = 0; i<c.size(); i++) {
			itm = c[i];
			outItem = 0;
			switch (itm->getCClassType()) {
			case SIMPLE:
				outItem = this->andSimpleSelf((SimpleCriteria&)*itm);
				break;

			case COMPOSITE:
				outItem = this->andCompositeSelf((CompositeCriteria&)*itm);
				break;
			}

			if (outItem != 0)
				((CompositeCriteria*)out)->add(outItem);
		}
	}
	return out;
}

Criteria* SimpleCriteria::andSimpleSelf(SimpleCriteria& c) {
	double d[2] = {c.leftVal, c.rightVal};
	Criteria* out = NULL;
	SimpleCriteria *tmp;
	
	if(this->variable == c.variable) {	
		if( !(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
				tmp = this->andValue(d);
				if(tmp != NULL) {
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
					return NULL;
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
						return NULL;
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
					return NULL;
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
		out = new CompositeCriteria();
		((CompositeCriteria*)out)->setOperator(AND);
		((CompositeCriteria*)out)->add(this);
		((CompositeCriteria*)out)->add(c.clone());
	}

	return out;
}

Criteria* SimpleCriteria::andCriteria(SimpleCriteria& c) {
	Criteria* out = NULL;
	double d[2] = {c.leftVal, c.rightVal};

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
					((SimpleCriteria*)out)->leftVal = c.leftVal;
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
					((SimpleCriteria*)out)->rightVal = c.rightVal;
				}
			}
		} if( !(this->leftInfinity) && (this->rightInfinity) ) { /* This interval is close on LEFT and open on RIGHT  */
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
				if(c.leftVal >= this->leftVal) {
					/*
						This   |-----------------
						c    	|-----------|
					*/
					out = (Criteria*)(c.clone());
				} else if(c.leftVal < this->leftVal) {
					/*
						This   |-----------------
						c    |-----------|
					*/
					out = c.clone();
					((SimpleCriteria*)out)->leftVal = this->leftVal;
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
					((SimpleCriteria*)out)->leftVal = this->leftVal;
				} else {
					/*
						This   			|--------
						c     --------|
					*/
					out = NULL;
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
						((SimpleCriteria*)out)->rightVal = this->rightVal;
					} else {
						/*
							This   ----|
							c    		 |------|
						*/
						out = NULL;
					}
				}
			} else if( !(c.leftInfinity) && (c.rightInfinity) ) { //c is close on left and open on right 101
				if( c.leftVal <= this->rightVal ) {
					/*
						This   --------|
						c    |-----------
					*/
					out = c.clone();
					((SimpleCriteria*)out)->rightVal = this->rightVal;
				} else {
					/*
						This   ---|
						c        	|-----
					*/
					out = NULL;
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
		out = new CompositeCriteria();
		((CompositeCriteria*)out)->setOperator(AND);
		((CompositeCriteria*)out)->add(this->clone());
		((CompositeCriteria*)out)->add(c.clone());
	}

	return out;
}

Criteria* SimpleCriteria::andCriteria(CompositeCriteria& c) {
	int i;
	Criteria *out, *outItem;
	Criteria *itm;

	if(c.logicOperator() == AND) {
		// SIMPLE & AND  = out(clone c), add clone SIMPLE to out
		out = c.clone();
		((CompositeCriteria*)out)->add(this->clone());
	} else {
		// SIMPLE & OR = SIMPLE & every sub OR
		out = new CompositeCriteria();
		((CompositeCriteria*)out)->setOperator(OR);
		
		for(i=0; i<c.size(); i++) {
			itm = c[i];
			outItem = 0;
			switch(itm->getCClassType()) {
				case SIMPLE:
					outItem = this->andCriteria((SimpleCriteria&)*itm);
				break;
				
				case COMPOSITE:
					outItem = this->andCriteria((CompositeCriteria&)*itm);
				break;
			}
			if(outItem != 0)
				((CompositeCriteria*)out)->add(outItem);
		}
	}
	return out;
}

Criteria* SimpleCriteria::operator &(Criteria& c) {
	Criteria *out;
	switch (c.getCClassType()){
		case SIMPLE:
			out = andCriteria((SimpleCriteria&)c);
			break;

		case COMPOSITE:
			out = andCriteria((CompositeCriteria&)c);
			break;
	}
	return out;
}

Criteria* SimpleCriteria::orCriteria(SimpleCriteria& c) {
	Criteria* out;
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
							((SimpleCriteria*)out)->rightVal = c.rightVal;
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
							((SimpleCriteria*)out)->rightVal = this->rightVal;
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
						
						((SimpleCriteria*)out)->leftVal = this->leftVal;
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
	} else usedComposite = true;


	if(usedComposite) {
		out = new CompositeCriteria();
		((CompositeCriteria*)out)->setOperator(OR);
		((CompositeCriteria*)out)->add(this->clone());
		((CompositeCriteria*)out)->add(c.clone());
	}
	
	return out;
}

CompositeCriteria* SimpleCriteria::orCriteria(CompositeCriteria& c) {
	CompositeCriteria* out = (CompositeCriteria*)c.clone();
	out->add(clone());

	return out;
}

/**
	Need to implement
*/
Criteria* SimpleCriteria::operator |(Criteria &c) {
	Criteria* out;
	switch (c.getCClassType()) {
		case SIMPLE:
			out = orCriteria((SimpleCriteria&)c);
			break;
		case COMPOSITE:
			out = orCriteria((CompositeCriteria&)c);
			break;
	}
	return out;
}

