#include <stdlib.h>
#include "nmath.h"
#include "SimpleCriteria.h"
#include "combinedcriteria.h"
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
	this->rightInfinity = 1;
	this->leftInfinity = 1;
	this->available = 1;
	this->type = type;
	this->variable = 0;
	this->leftVal = 0;
	this->rightVal = 0;
}

SimpleCriteria::SimpleCriteria(int type, char var, float lval, float rval, 
										char leftInfinity, char rightInfinity) {
	this->type = type;
	this->variable = var;
	this->leftVal = lval;
	this->rightVal = rval;
	this->rightInfinity = rightInfinity;
	this->leftInfinity = leftInfinity;
	this->available = 1;
}

SimpleCriteria* SimpleCriteria::clone() {
	SimpleCriteria *out;
	out = new SimpleCriteria(type, variable, leftVal, rightVal, leftInfinity, rightInfinity);
	return out;
}

int SimpleCriteria::check(float values) {
	int result = FALSE;
	
	if( (this->leftInfinity) && (this->rightInfinity) )
		return TRUE;
		
	if( this->leftInfinity ) {
		/** HERE we don't need to take care of leftVal */
		switch(this->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(values < this->rightVal)
					result = TRUE;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(values <= this->rightVal)
					result = TRUE;
			break;
		}
	}else if ( this->rightInfinity ) {
		/** HERE we don't need to take care of rightVal */
		switch(this->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(this->leftVal < values)
					result = TRUE;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(this->leftVal <= values)
					result = TRUE;
			break;
		}
	}else{
		switch(this->type){
			case GT_LT:
				// leftVal < x < rightVal
				if( (this->leftVal < values) && (values < this->rightVal))
					result = TRUE;
			break;
			
			case GT_LTE:
				// leftVal < x <= rightVal
				if( (this->leftVal < values) && (values <= this->rightVal))
					result = TRUE;
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if( (this->leftVal <= values) && (values < this->rightVal))
					result = TRUE;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if( (this->leftVal <= values) && (values <= this->rightVal))
					result = TRUE;
			break;
		}
	}
	
	return result;
}

Criteria* SimpleCriteria::and(const float& values ){
	SimpleCriteria *outInterval = new SimpleCriteria();
	
	outInterval->available = 1;
	outInterval->leftInfinity = 0;
	outInterval->rightInfinity = 0;

	if( this->leftInfinity && this->rightInfinity ) {
		outInterval->leftVal = values[0];
		outInterval->rightVal = values[1];
		outInterval->type = GTE_LTE;
		return outInterval;
	}
		
	if( this->leftInfinity ) {
		/** HERE we don't need to take care of leftVal */
		outInterval->leftVal = values[0];
		switch(this->type){
			case GT_LT:
			case GTE_LT:
				// x < rightVal
				if(this->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				outInterval->type = GTE_LT; //TODO: need to test here
				if(values[1] < this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
					//outInterval->rightVal = this->rightVal - epsilon;
			break;
			
			case GT_LTE:
			case GTE_LTE:
				// x <= rightVal
				if(this->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				
				outInterval->type = GTE_LTE; //TODO: need to test here
				if(values[1] <= this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
			break;
		}
	}else if ( this->rightInfinity ) {
		/** HERE we don't need to take care of rightVal */
		outInterval->rightVal = values[1];
		switch(this->type){
			case GT_LT:
			case GT_LTE:
				// leftVal < x
				if(this->leftVal >= values[1]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				outInterval->type = GT_LTE; //TODO: need to test here
				if(this->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
			break;
			
			case GTE_LT:
			case GTE_LTE:
				// leftVal <= x
				if(this->leftVal > values[1]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				
				outInterval->type = GTE_LTE; //TODO: need to test here
				if(this->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
			break;
		}
	}else{
		switch(this->type){
			case GT_LT:
				// leftVal < x < rightVal
				if(this->leftVal >= values[1] || this->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				
				if(this->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
				
				if(values[1] < this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
					
			break;
			
			case GT_LTE:
				if(this->leftVal >= values[1] || this->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				
				if(this->leftVal < values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
					
				if(values[1] <= this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
					
			break;
			
			case GTE_LT:
				// leftVal <= x < rightVal
				if(this->leftVal > values[1] || this->rightVal <= values[0]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				
				if(this->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
					
				if(values[1] < this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
			break;
			
			case GTE_LTE:
				// leftVal <= x <= rightVal
				if(this->leftVal > values[1] || this->rightVal < values[0]){
					//return empty set, available bit set to FALSE
					outInterval->available = 0;
					return;
				}
				
				if(this->leftVal <= values[0])
					outInterval->leftVal = values[0];
				else 
					outInterval->leftVal = this->leftVal;
					
				if(values[1] <= this->rightVal)
					outInterval->rightVal = values[1];
				else 
					outInterval->rightVal = this->rightVal;
			break;
		}
	}

	return outInterval;
}

Criteria* SimpleCriteria::and(const SimpleCriteria &c) {
	Criteria* out = NULL;
	Criteria* outCriteria;
	float d[2] = {c.leftVal, c.rightVal};

	if(this->variable == c.variable) {	
		if( !(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
			if( !(c.leftInfinity) && !(c.rightInfinity) ) { //c is closed
				out = this->and(d);
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
					out = c.clone();
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
				out = this->clone()
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
		out = new CombinedCriteria();
		out &= this;
		out &= c;
	}

	return out;
}

Criteria* SimpleCriteria::and(const CombinedCriteria& cb) {
	int i, j;
	Criteria* tmp;
	CombinedCriteria *out;
	for(i=0; i < cb->size(); i++) {
		if( this->variable == cb[i]->variable ){
			tmp = this->and(cb[i]);
			if(tmp != NULL) {
				//TODO: Copy cb to out except element at i
				out = new CombinedCriteria();
				for(j=0; j < cb->size(); j++) {
					if(j != i)
						out->add(cb[j].clone());
				}
				out->add(tmp);
				return out;
			} else
				return NULL;
		}
	}

	/*
		We got here because this criteria has variable that not same as vaiable of any criteria in CombinedCriteria
	*/
	out = new CombinedCriteria();
	out = cb->clone();
	out &= this;
	return out;
}

Criteria* SimpleCriteria::and(const CompositeCriteria& inputComp) {
	CombinedCriteria *cbOut;
	CompositeCriteria *comp1;
	/**
		c1 and (c2 or  c3) = (c1 and c2) or (c1 and c3)
	*/
	comp1 = new CompositeCriteria();
	for(i=0; i<inputComp->size(); i++) {
		cb = inputComp[i];
		if( (cbOut = this->and(cb)) != NULL ) {
			comp1 += cbOut;
		}
	}	
	return comp1;
}

Criteria* SimpleCriteria::operator &(const Criteria &c) {
	Criteria *out = and(c);
	return out;
}


/**
	@return 
		FALSE: if it is contradiction
		TRUE: 
*/
int andTwoCriteria(const void *c1, const void *c2, OutBuiltCriteria *out){
	int objTypeLeft = *((char*)c1);
	int objTypeRight = *((char*)c2);
	int i, result = FALSE;
	float d[2];
	Criteria *interval, *cr;
	CombinedCriteria *cb, *comb1, *comb2;
	CompositeCriteria *inputComp, *outComp, *comp1, *comp2;
	void *tmp;
	OutBuiltCriteria outTmp;
	
	if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		result = andTwoSimpleCriteria((const Criteria*)c1, (const Criteria*)c2, out);
	} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
		
	} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
		
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
		
	} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
		
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
		inputComp = (CompositeCriteria*)c1;
		comp1 = newCompositeInterval();
		for(i=0; i<inputComp->size; i++) {
			cb = inputComp->list[i];
			outTmp.cr = NULL;
			if( andTwoCriteria((Criteria*)c2, cb, &outTmp) ){
				if(comp1->size >= comp1->loggedSize){
					comp1->loggedSize += 5;
					tmp = realloc(comp1->list, sizeof(CombinedCriteria*) * comp1->loggedSize);
					comp1->list = (CombinedCriteria**)(tmp==NULL?comp1->list:tmp);
				}
				comp1->list[comp1->size++] = (CombinedCriteria*)(outTmp.cr);
			}
		}
		result = TRUE;
		out->cr = comp1;
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
		comb1 = (CombinedCriteria*)c2;
		comp2 = (CompositeCriteria*)c1;
		outComp = newCompositeInterval();
		outComp->loggedSize = comp2->size;
		outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
		for(i=0; i<comp2->size; i++) {
			cb = comp2->list[i];
			andTwoCriteria(comb1, cb, &outTmp);

			if(outComp->size >= outComp->loggedSize) {
				outComp->loggedSize += 5;
				tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
				outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
			}

			outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
			outTmp.cr = NULL;
		}
		out->cr = outComp;
		result = TRUE;
	} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
		comp1 = (CompositeCriteria*)c1;
		comp2 = (CompositeCriteria*)c2;
		outComp = newCompositeInterval();
		outComp->loggedSize = comp2->size;
		outComp->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*) * outComp->loggedSize);
		for(i=0; i<comp2->size; i++) {
			cb = comp2->list[i];
			andTwoCriteria(comp1, cb, &outTmp);

			if(outComp->size >= outComp->loggedSize) {
				outComp->loggedSize += 5;
				tmp = realloc(outComp->list, sizeof(CombinedCriteria*) * outComp->loggedSize);
				outComp->list = (tmp==NULL)?outComp->list:((CombinedCriteria**)tmp);
			}

			outComp->list[(outComp->size)++] = (CombinedCriteria*)outTmp.cr;
			outTmp.cr = NULL;
		}
		out->cr = outComp;
		result = TRUE;
	}
	return result;
}

/**
	Need to implement
*/
CompositeCriteria Criteria::operator |(const Criteria *c) {
	float d[2];
	Criteria *interval;
	CombinedCriteria* cb;
	CompositeCriteria* out = new CompositeCriteria();

	if(this->variable == c->variable) {
		if( !(this->leftInfinity) && !(this->rightInfinity) ) { //If this interval is close
			if( !(c->leftInfinity) && !(c->rightInfinity) ) { //c is close

			} else if( !(c->leftInfinity) && c->rightInfinity ) { // c is close on LEFT and open on RIGHT

			} else if( c->leftInfinity && !(c->rightInfinity) ) { //c is open on LEFT and close on RIGHT

			}
		} if( (this->flag | 0x05) == 0x05 ) { /* This interval is close on RIGHT and open on LEFT  */

		} if( (this->flag | 0x06) == 0x06 ) { /* This interval is close on LEFT and open on RIGHT  */

		}

		if( ((this->flag & 0x05)==0x05) && ((c->flag & 0x06)==0x06) ) {
			if(this->rightVal >= c->leftVal) {
				interval = new Criteria(this->variable, -9999, 9999, RIGHT_INF, LEFT_INF);
				cb = new CombinedCriteria();
				cb->and(interval);
				out->or(cb);
			} else {
				interval = this->clone();
				cb = new CombinedCriteria();
				cb->and(interval);
				out->or(cb);

				interval = c->clone();
				cb = new CombinedCriteria();
				cb->and(interval);
				out->or(cb);
			}
		} else if( ((this->flag & 0x05)==0x05) && ((c->flag & 0x05)==0x05) ) {
			if(this->rightVal >= c->rightVal) {
				interval = this->clone();
			} else {
				interval = c->clone();
			}
			cb = new CombinedCriteria();
			cb->and(interval);
			out->or(cb);

		} else if( ((this->flag & 0x05)==0x05) && ((c->flag & 0x05)==0x05) ) {

		}


		interval = newCriteria(GT_LT, 'x', 0, 0, FALSE, FALSE);
		d[0] = c2->leftVal;
		d[1] = c2->rightVal;
		c1->fgetInterval(c1, d, 1, (void*)interval);
		if( (interval->flag & AVAILABLE) == 0){
			free(interval);
			((CompositeCriteria*)(out->cr))->list = (CombinedCriteria**)malloc(sizeof(CombinedCriteria*)*2);
			((CompositeCriteria*)(out->cr))->loggedSize = 2;
			((CompositeCriteria*)(out->cr))->size = 2;
			
			((CompositeCriteria*)(out->cr))->list[0] = newCombinedInterval();
			((CompositeCriteria*)(out->cr))->list[0]->list = (Criteria**)malloc(sizeof(Criteria*));
			((CompositeCriteria*)(out->cr))->list[0]->loggedSize = 1;
			((CompositeCriteria*)(out->cr))->list[0]->size = 1;
			((CompositeCriteria*)(out->cr))->list[0]->list[0] = (Criteria*)malloc(sizeof(Criteria));
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->objectType = SIMPLE_CRITERIA;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->type = c1->type;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->variable = c1->variable;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->leftVal = c1->leftVal;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->rightVal = c1->rightVal;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->fcheck = isInInterval;
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->fgetInterval = getInterval;
			//set available bit, unset left and right infinity
			((CompositeCriteria*)(out->cr))->list[0]->list[0]->flag = (AVAILABLE) | (c1->flag | 0xfc);
			
			((CompositeCriteria*)(out->cr))->list[1] = newCombinedInterval();
			((CompositeCriteria*)(out->cr))->list[1]->list = (Criteria**)malloc(sizeof(Criteria*));
			((CompositeCriteria*)(out->cr))->list[1]->loggedSize = 1;
			((CompositeCriteria*)(out->cr))->list[1]->size = 1;
			((CompositeCriteria*)(out->cr))->list[1]->list[0] = (Criteria*)malloc(sizeof(Criteria));
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->objectType = SIMPLE_CRITERIA;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->type = c2->type;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->variable = c2->variable;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->leftVal = c2->leftVal;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->rightVal = c2->rightVal;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->fcheck = isInInterval;
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->fgetInterval = getInterval;
			//set available bit, unset left and right infinity
			((CompositeCriteria*)(out->cr))->list[1]->list[0]->flag = AVAILABLE | (c2->flag | 0xfc);
			return TRUE;
		}
		
		interval->leftVal = (c1->leftVal < c2->leftVal)?c1->leftVal:c2->leftVal;
		interval->rightVal = (c1->rightVal < c2->rightVal)?c2->rightVal:c1->rightVal;
		
		out->cr = interval;
	}else{
		interval = this->clone();
		cb = new CombinedCriteria();
		cb->and(interval);
		out->or(cb);

		interval = c->clone();
		cb = new CombinedCriteria();
		cb->and(interval);
		out->or(cb);
	}
	
	return out;
}

