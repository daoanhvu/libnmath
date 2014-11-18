#include <stdlib.h>
#include "nmath.h"
#include "criteria.h"

#ifdef _TARGET_HOST_ANDROID
	#include <jni.h>
	#include <android/log.h>
	#define LOG_TAG "NativeFunction"
	#define LOG_LEVEL 10
	#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
	#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

extern int gErrorCode;

int andTwoSimpleCriteria(const Criteria* c1, const Criteria* c2, OutBuiltCriteria* out);
int orTwoSimpleCriteria(const Criteria* c1, const Criteria* c2, OutBuiltCriteria* out);
int andTwoCriteria(const void* c1, const void* c2, OutBuiltCriteria* out);

Criteria::Criteria() {
	this->rightInfinity = 1;
	this->leftInfinity = 1;
	this->available = 1;
	this->type = type;
	this->variable = 0;
	this->leftVal = 0;
	this->rightVal = 0;
}

Criteria::Criteria(int type, char var, float lval, float rval, 
										char leftInfinity, char rightInfinity) {
	this->type = type;
	this->variable = var;
	this->leftVal = lval;
	this->rightVal = rval;
	this->rightInfinity = rightInfinity;
	this->leftInfinity = leftInfinity;
	this->available = 1;
}

int Criteria::check(float values) {
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

Criteria& Criteria::and(const float& values ){
	Criteria *outInterval = new Criteria();
	
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

CombinedCriteria& Criteria::operator &(const Criteria &c) {
	CombinedCriteria* out = NULL;
	Criteria* outCriteria;
	float d[2] = {c->leftVal, c->rightVal};

	if(this->variable == c->variable) {	
		if( !(this->leftInfinity) && !(this->rightInfinity)) { /* If this interval is closed*/
			if( !(c->leftInfinity) && !(c->rightInfinity) ) { //c is closed
				outCriteria = this->and(d);
				out = new CombinedCriteria();
				out->add(outCriteria);
			} else if( !(c->leftInfinity) && (c->rightInfinity) ) { //c is close on left and open on right 101
				if(this->leftVal >= c->leftVal) {
					/*
						This   |--------|
						c    |-----------
					*/
					out = new CombinedCriteria();
					out->add(this->clone());
				} else if( (this->leftVal < c->leftVal) && (this->rightVal >= c->leftVal) ) {
					/*
						This   |--------|
						c        |---------
					*/
					out = new CombinedCriteria();
					outCriteria = this->clone();
					outCriteria->leftVal = c->leftVal;
					out->add(outCriteria);
				}
			} else if( (c->leftInfinity) && !(c->rightInfinity) ) { //c is close on right and open on left 110
				if( this->rightVal <= c->rightVal) {
					/*
						This   |--------|
						c     ---------------|
					*/
					out = new CombinedCriteria();
					out->add(this->clone());
				} else if( (c->rightVal<=this->rightVal) && (c->rightVal > this->leftVal) )  {
					/*
						This   |--------|
						c     --------|
					*/
					out = new CombinedCriteria();
					outCriteria = this->clone();
					outCriteria->rightVal = c->rightVal;
					out->add(outCriteria);
				}
			}
		} if( !(this->leftInfinity) && (this->rightInfinity) ) { /* This interval is close on LEFT and open on RIGHT  */
			if( !(c->leftInfinity) && !(c->rightInfinity) ) { //c is closed
				if(c->leftVal >= this->leftVal) {
					/*
						This   |-----------------
						c    	|-----------|
					*/
					out = new CombinedCriteria();
					out->add(c->clone());
				} else if(c->leftVal < this->leftVal) {
					/*
						This   |-----------------
						c    |-----------|
					*/
					out = new CombinedCriteria();
					outCriteria = c->clone();
					outCriteria->leftVal = this->leftVal;
					out->add(outCriteria;
				}
			} else if( !(c->leftInfinity) && (c->rightInfinity) ) { //c is close on left and open on right 101
				if() { DEN DAY
					/*
						This   |--------|
						c    |-----------
					*/
					
				} else if(  ) {
					/*
						This   |--------|
						c        |---------
					*/
					
				}
			} else if( (c->leftInfinity) && !(c->rightInfinity) ) { //c is close on right and open on left 110
				if( ) {
					/*
						This   |--------|
						c     ---------------|
					*/
					
				} else if(  )  {
					/*
						This   |--------|
						c     --------|
					*/
					
				}
			}
		} if( (this->leftInfinity) && !(this->rightInfinity) ) { /* This interval is close on RIGHT and open on LEFT  */
			if( !(c->leftInfinity) && !(c->rightInfinity) ) { //c is closed
				if() {
					/*
						This   |-----------------
						c    	|-----------|
					*/
					
				} else if() {
					/*
						This   |-----------------
						c    |-----------|
					*/
				}
			} else if( !(c->leftInfinity) && (c->rightInfinity) ) { //c is close on left and open on right 101
				if() {
					/*
						This   |--------|
						c    |-----------
					*/
					
				} else if(  ) {
					/*
						This   |--------|
						c        |---------
					*/
					
				}
			} else if( (c->leftInfinity) && !(c->rightInfinity) ) { //c is close on right and open on left 110
				if( ) {
					/*
						This   |--------|
						c     ---------------|
					*/
					
				} else if(  )  {
					/*
						This   |--------|
						c     --------|
					*/
					
				}
			}
		}
	} else {
		out = new CombinedCriteria();
		out &= this;
		out &= c;
	}

	return out;
}

CombinedCriteria& Criteria::operator &(const CombinedCriteria& cb) {
	int i = 0;
	CombinedCriteria* out;
	Criteria* c;
	for(i=0; i < cb->size(); i++) {
		if( this->variable == cb[i]->variable ){
			interval = this & cb[i];
			if( (interval != NULL) && ((interval->flag & AVAILABLE) == AVAILABLE) {
				out = new CombinedCriteria();
				out = cb;
				out[i] = interval;
				delete interval;
				return out;
			}else{
				if(interval != NULL) delete interval;
				/** ERROR: AND two contracting criteria */
				return NULL;
			}
			break;
		}
	}

	/*
		We got here because this criteria has variable that not same as vaiable of any criteria in CombinedCriteria
	*/
	out = new CombinedCriteria();
	out = cb;
	out &= this;
	return out;
}

CompositeCriteria& Criteria::operator &(const CompositeCriteria& inputComp) {
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
		comb1 = (CombinedCriteria*)c1;
		comp2 = (CompositeCriteria*)c2;
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
CompositeCriteria Criteria::or(const Criteria *c) {
	float d[2];
	Criteria *interval;
	CombinedCriteria* cb;
	CompositeCriteria* out = new CompositeCriteria();

	if(this->variable == c->variable) {

		/*
			If this interval is close
		*/
		if( (this->flag | 0x04) == 0x04) {
			if( (c->flag | 0x04) == 0x04) {
			} else if( (c->flag | 0x05) == 0x05) {
			} else if( (c->flag | 0x06) == 0x06) {
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

/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be greater than or equals 2
 *	@param epsilon
 */
FData* generateOneUnknows(NMAST* exp, const char *variables /*1 in length*/,
						const Criteria *c, const float *bd, int bdlen, float epsilon){
	int elementOnRow;
	Criteria out1;
	float right1, lastX;
	float y;
	float values[1];
	void *tmpP;
	FData *mesh = NULL;
	DParamF param;
	
	out1.objectType = SIMPLE_CRITERIA;
	out1.type = GT_LT;
	out1.variable = variables[0];
	out1.leftVal = 0;
	out1.rightVal = 0;
	out1.fcheck = isInInterval;
	out1.fgetInterval = getInterval;
	out1.flag = AVAILABLE;
	
	getInterval(c, bd, 0, &out1);
		
	if( (out1.flag && AVAILABLE) == 0){
		return NULL;
	}
	
	param.t = exp;
	param.variables[0] = variables[0];
	param.values = values;
	param.error = 0;
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 2;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (float*)malloc(sizeof(float) * mesh->loggedSize);
	mesh->rowCount = 0;
	mesh->loggedRowCount = 1;
	mesh->rowInfo = (int*)malloc(sizeof(int));
	
	param.values[0] = out1.leftVal;
	if(out1.type == GT_LT || out1.type == GT_LTE)
		param.values[0] = out1.leftVal + epsilon;
		
	right1 = out1.rightVal;
	if(out1.type == GT_LT || out1.type == GTE_LT)
		right1 = out1.rightVal - epsilon;
		
	elementOnRow = 0;
	while(param.values[0] <= right1){
		calcF_t((void*)&param);
		y = param.retv;
		if(mesh->dataSize >= mesh->loggedSize - 2){
			mesh->loggedSize += 20;
			tmpP = realloc(mesh->data, sizeof(float) * mesh->loggedSize);
			if(tmpP != NULL)
				mesh->data = (float*)tmpP;
		}
		mesh->data[mesh->dataSize++] = param.values[0];
		mesh->data[mesh->dataSize++] = y;
		elementOnRow++;
		lastX = param.values[0];
		param.values[0] += epsilon;
	}

	if(lastX < right1 && param.values[0]>right1){
		param.values[0] = right1;
		calcF_t((void*)&param);
		y = param.retv;
		if(mesh->dataSize >= mesh->loggedSize - 2){
			mesh->loggedSize += 4;
			tmpP = realloc(mesh->data, sizeof(float) * mesh->loggedSize);
			if(tmpP != NULL)
				mesh->data = (float*)tmpP;
		}
		mesh->data[mesh->dataSize++] = param.values[0];
		mesh->data[mesh->dataSize++] = y;
		elementOnRow++;
	}

	mesh->rowInfo[mesh->rowCount++] = elementOnRow;
	return mesh;
}


/**
 * 	This get a continuous 3D space
 *	@param exp
 *	@param bd
 *	@param bdlen MUST be 4 
 *	@param epsilon
 */
FData* generateTwoUnknowsFromCombinedCriteria(NMAST* exp, const char *variables, const CombinedCriteria *c, const float *bd, 
													int bdlen, float epsilon) {
	int elementOnRow;
	Criteria out1, out2;
	float right1, left2, right2;
	float y;
	float values[2];
	void *tmpP;
	FData *mesh = NULL;
	DParamF param;
	
	out1.objectType = SIMPLE_CRITERIA;
	out1.flag = AVAILABLE;
	out1.type = GT_LT;
	out1.variable = variables[0];
	out1.leftVal = 0;
	out1.rightVal = 0;
	out1.fcheck = isInInterval;
	out1.fgetInterval = getInterval;
	
	out2.objectType = SIMPLE_CRITERIA;
	out2.flag = AVAILABLE;
	out2.type = GT_LT;
	out2.variable = variables[1];
	out2.leftVal = 0;
	out2.rightVal = 0;
	out2.fcheck = isInInterval;
	out2.fgetInterval = getInterval;
	
	getInterval(c->list[0], bd, 0, &out1);
	getInterval(c->list[1], bd+2, 0, &out2);
		
	if( (out1.flag & AVAILABLE) == 0 || (out2.flag & AVAILABLE) == 0){
		return NULL;
	}
	
	param.t = exp;
	param.variables[0] = variables[0];
	param.variables[1] = variables[1];
	param.values = values;
	param.error = 0;
	
	mesh = (FData*)malloc(sizeof(FData));
	mesh->dimension = 3;
	mesh->loggedSize = 20;
	mesh->dataSize = 0;
	mesh->data = (float*)malloc(sizeof(float) * mesh->loggedSize);
	mesh->loggedRowCount = 0;
	mesh->rowCount = 0;
	mesh->rowInfo= NULL;
#ifdef DEBUG
	incNumberOfDynamicObject();
	incNumberOfDynamicObject();
#endif

	param.values[0] = out1.leftVal;
	if(out1.type == GT_LT || out1.type == GT_LTE)
		param.values[0] = out1.leftVal + epsilon;
		
	right1 = out1.rightVal;
	if(out1.type == GT_LT || out1.type == GTE_LT)
		param.values[0] = out1.leftVal - epsilon;
	
	left2 = out2.leftVal;
	if(out2.type == GT_LT || out2.type == GT_LTE)
		left2 = out2.leftVal + epsilon;
		
	right2 = out2.rightVal;
	if(out2.type == GT_LT || out2.type == GTE_LT)
		right2 = out2.leftVal - epsilon;
	
	while(param.values[0] <= right1){
		param.values[1] = left2;
		elementOnRow = 0;
		while(param.values[1] <= right2){
			calcF_t((void*)&param);
			y = param.retv;
			if(mesh->dataSize >= mesh->loggedSize - 3){
				mesh->loggedSize += 20;
				tmpP = realloc(mesh->data, sizeof(float) * mesh->loggedSize);
				if(tmpP != NULL)
					mesh->data = (float*)tmpP;
			}
			mesh->data[mesh->dataSize++] = param.values[0];
			mesh->data[mesh->dataSize++] = param.values[1];
			mesh->data[mesh->dataSize++] = y;
			param.values[1] += epsilon;
			elementOnRow++;
		}
		if(mesh->rowCount >= mesh->loggedRowCount){
			mesh->loggedRowCount += 10;
			tmpP = realloc(mesh->rowInfo, sizeof(int) * mesh->loggedRowCount);
			if(tmpP != NULL)
				mesh->rowInfo = (int*)tmpP;
		}
		
		mesh->rowInfo[mesh->rowCount++] = elementOnRow;
		param.values[0] += epsilon;
	}
	return mesh;
}

ListFData *getValueRangeForOneVariableF(Function *f, const float *bd, int bdlen, float epsilon){
	ListFData *lst = NULL;
	FData *sp;
	int i;

	lst = (ListFData*)malloc(sizeof(ListFData));
	lst->loggedSize = f->prefix->size;
	lst->size = 0;
	lst->list = (FData**)malloc(sizeof(FData*) * f->prefix->size);
	for(i=0; i<f->prefix->size; i++){
		sp = generateOneUnknows(f->prefix->list[i], f->variable, (const Criteria*)(f->criterias->list[i]), bd, 2, epsilon);
		if(sp != NULL)
			lst->list[(lst->size)++] = sp;
	}

	return lst;
}

ListFData *getSpaces(Function *f, const float *bd, int bdlen, float epsilon) {
	ListFData *lst = NULL;
	FData *sp;
	CombinedCriteria *comb;
	CompositeCriteria *composite;
	Criteria c;
	OutBuiltCriteria outCriteria;
	int i, j, outCriteriaType;
	
	outCriteria.cr = NULL;
	switch(f->valLen){
		case 1:
			lst = (ListFData*)malloc(sizeof(ListFData));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
			if(f->domain == NULL){
				c.objectType = SIMPLE_CRITERIA;
				c.flag = 0x07;// LEFT INFINITY & RIGHT INFINITY & AVAILABLE;
				c.type = GTE_LTE;
				c.variable = f->variable[0];
				c.leftVal = bd[0];
				c.rightVal = bd[1];
				c.fcheck = isInInterval;
				c.fgetInterval = getInterval;
				
				sp = generateOneUnknows(f->prefix->list[0], f->variable, &c, bd, 2, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
					lst->list[0] = sp;
				}
			} else {
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
				outCriteriaType = *((char*)(outCriteria.cr));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						sp = generateOneUnknows(f->prefix->list[0], f->variable, (Criteria*)(outCriteria.cr), bd, 2, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
							lst->list[0] = sp;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						}
						free(outCriteria.cr);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					break;
					
					case COMBINED_CRITERIA:
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(outCriteria.cr);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						lst->loggedSize = composite->size;
						lst->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateOneUnknows(f->prefix->list[0], f->variable, comb->list[0], bd, 2, epsilon);
							if(sp != NULL){
								lst->list[lst->size++] = sp;
							}
							for(j=0; j<comb->size; j++){
								free(comb->list[j]);
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(j+1+2);
#endif
						}
						free(composite->list);
						free(composite);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
				}//end switch
			}
		break;
		
		case 2:
			lst = (ListFData*)malloc(sizeof(ListFData));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
			if(f->domain == NULL){
				comb = newCombinedInterval();
				comb->loggedSize = 2;
				comb->size = 2;
				comb->list = (Criteria **)malloc(sizeof(Criteria *) * comb->loggedSize);
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				comb->list[0] = newCriteria(GT_LT, f->variable[0], bd[0], bd[1], TRUE, TRUE);
				comb->list[1] = newCriteria(GT_LT, f->variable[1], bd[2], bd[3], TRUE, TRUE);
				sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
					lst->list[0] = sp;
				}
				for(i=0; i<comb->size; i++) {
					free(comb->list[i]);
				}
				free(comb->list);
				free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
#endif
			} else {
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
				outCriteriaType = *((char*)(outCriteria.cr));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						free(outCriteria.cr);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					break;
					
					case COMBINED_CRITERIA:
						comb = (CombinedCriteria*)(outCriteria.cr);
						sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
							lst->list[0] = sp;
						}
						for(i=0; i<comb->size; i++){
							free(comb->list[i]);
						}
						free(comb->list);
						free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
#endif
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(outCriteria.cr);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						lst->loggedSize = composite->size;
						lst->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
							if(sp != NULL)
								lst->list[lst->size++] = sp;
							for(j=0; j<comb->size; j++){
								free(comb->list[j]);
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(j+1+2);
#endif
						}
						free(composite->list);
						free(composite);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
				}
			}
		break;
		
		case 3:
		break;
		
		default:
		break;
	}
	return lst;
}

ListFData *getSpaces2(Function *f, const float *bd, int bdlen, float epsilon){
	ListFData *lst = NULL;
	FData *sp;
	CombinedCriteria *comb;
	CompositeCriteria *composite;
	Criteria c;
	OutBuiltCriteria outCriteria;
	int i, j, outCriteriaType;
	
	outCriteria.cr = NULL;
	switch(f->valLen){
		case 1:
			lst = (ListFData*)malloc(sizeof(ListFData));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
			if(f->domain == NULL){
				c.objectType = SIMPLE_CRITERIA;
				c.flag = AVAILABLE;
				c.type = GT_LT;
				c.variable = f->variable[0];
				c.leftVal = bd[0];
				c.rightVal = bd[1];
				c.fcheck = isInInterval;
				c.fgetInterval = getInterval;
				
				sp = generateOneUnknows(f->prefix->list[0], f->variable, &c, bd, 2, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
					lst->list[0] = sp;
				}
			} else {
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
				outCriteriaType = *((char*)(outCriteria.cr));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						sp = generateOneUnknows(f->prefix->list[0], f->variable, (Criteria*)(outCriteria.cr), bd, 2, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
							lst->list[0] = sp;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						}
						free(outCriteria.cr);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					break;
					
					case COMBINED_CRITERIA:
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(outCriteria.cr);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						lst->loggedSize = composite->size;
						lst->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateOneUnknows(f->prefix->list[0], f->variable, comb->list[0], bd, 2, epsilon);
							if(sp != NULL){
								lst->list[lst->size++] = sp;
							}
							for(j=0; j<comb->size; j++){
								free(comb->list[j]);
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(j+1+2);
#endif
						}
						free(composite->list);
						free(composite);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
				}//end switch
			}
		break;
		
		case 2:
			lst = (ListFData*)malloc(sizeof(ListFData));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
			if(f->domain == NULL){
				comb = newCombinedInterval();
				comb->loggedSize = 2;
				comb->size = 2;
				comb->list = (Criteria **)malloc(sizeof(Criteria *) * comb->loggedSize);
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
				comb->list[0] = newCriteria(GT_LT, f->variable[0], bd[0], bd[1], TRUE, TRUE);
				comb->list[1] = newCriteria(GT_LT, f->variable[1], bd[2], bd[3], TRUE, TRUE);
				sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
				if(sp != NULL){
					lst->loggedSize = 1;
					lst->size = 1;
					lst->list = (FData**)malloc(sizeof(FData*));
					lst->list[0] = sp;
				}
				for(i=0; i<comb->size; i++) {
					free(comb->list[i]);
				}
				free(comb->list);
				free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
#endif
			} else {
				buildCompositeCriteria(f->domain->list[0], f->variable, f->valLen, &outCriteria);
				outCriteriaType = *((char*)(outCriteria.cr));
				switch(outCriteriaType){
					case SIMPLE_CRITERIA:
						free(outCriteria.cr);
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
					break;
					
					case COMBINED_CRITERIA:
						comb = (CombinedCriteria*)(outCriteria.cr);
						sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
						if(sp != NULL){
							lst->loggedSize = 1;
							lst->size = 1;
							lst->list = (FData**)malloc(sizeof(FData*));
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
							lst->list[0] = sp;
						}
						for(i=0; i<comb->size; i++){
							free(comb->list[i]);
						}
						free(comb->list);
						free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(i+1+2);
#endif
					break;
					
					case COMPOSITE_CRITERIA:
						composite = (CompositeCriteria*)(outCriteria.cr);
						lst->list = (FData**)malloc(sizeof(FData*) * composite->size );
						lst->loggedSize = composite->size;
						lst->size = 0;
#ifdef DEBUG
	incNumberOfDynamicObject();
#endif
						for(i=0; i<composite->size; i++){
							comb = composite->list[i];
							sp = generateTwoUnknowsFromCombinedCriteria(f->prefix->list[0], f->variable, comb, bd, 4, epsilon);
							if(sp != NULL)
								lst->list[lst->size++] = sp;
							for(j=0; j<comb->size; j++){
								free(comb->list[j]);
							}
							free(comb->list);
							free(comb);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(j+1+2);
#endif
						}
						free(composite->list);
						free(composite);
#ifdef DEBUG
	descNumberOfDynamicObjectBy(2);
#endif
					break;
				}
			}
		break;
		
		case 3:
		break;
		
		default:
		break;
	}
	return lst;
}

/**
	Get the normal vector at each point in values
*/
double *getNormalVectors(Function *f, double *values, int numOfPoint) {
	DParam d;
	DParam rp;
	NMAST *dx;
	//NMAST *dy;
	//NMAST *dz;

	switch(f->valLen){
		case 1:
			d.t = f->prefix->list[0];
			d.error = 0;
			d.returnValue = NULL;
			d.variables[0] = f->variable[0];
	
			derivative(&d);
			if(d.error != NMATH_NO_ERROR) {
				gErrorCode = d.error;
				return NULL;
			}
			dx = d.returnValue;

			rp.error = 0;
			rp.t = dx;
			rp.values = values;
			rp.variables[0] = f->variable[0];
			calc_t((void*)&rp);

			/**
				The tangen of the f at (x0) is:
				y = dx(x0)(x-x0)+y0 

				Note that if above calc_t produces a ERROR_DIV_BY_ZERO then its very likely the tangen is 
				perspendicular X axis and it have form as x = x0
			*/
			if(rp.error == NMATH_NO_ERROR){

			}else if(rp.error == ERROR_DIV_BY_ZERO){

			}
			//return rp.retv;
		break;

		case 2:
			d.t = f->prefix->list[0];
			d.error = 0;
			d.returnValue = NULL;
			d.variables[0] = f->variable[0];
			d.variables[1] = f->variable[1];
	
			derivative(&d);
			if(d.error != NMATH_NO_ERROR) {
				gErrorCode = d.error;
				return NULL;
			}
			dx = d.returnValue;
		break;

		case 3:
		break;
	}
	
	return NULL;
}

void buildCriteria(Function *f) {
	int i;
	OutBuiltCriteria outResult;

	f->criterias = (ListCriteria*)malloc(sizeof(ListCriteria));
	f->criterias->loggedSize = f->prefix->loggedSize;
	f->criterias->size = 0;
	f->criterias->list = (void**)malloc(sizeof(void*) * f->criterias->loggedSize);
	for(i=0; i<f->prefix->size; i++){
		outResult.cr = NULL;
		if(f->domain==NULL || i>=f->domain->size)
			buildCompositeCriteria(NULL, f->variable, f->valLen, &outResult);
		else
			buildCompositeCriteria(f->domain->list[i], f->variable, f->valLen, &outResult);
		f->criterias->list[(f->criterias->size)++] = outResult.cr;
	}
}