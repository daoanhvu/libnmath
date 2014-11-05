#include "compositecriteria.h"

CompositeCriteria::CompositeCriteria(){
	list = NULL;
	loggedSize = 0;
	size = 0;
}

CompositeCriteria::~CompositeCriteria(){

}

void CompositeCriteria::release() {
	
}

CombinedCriteria CompositeCriteria::operator [](int index){
	if(list == NULL || index >= size) return NULL;

	return list[index];
}

int CompositeCriteria::isInInterval(const float* values) {
	int i;
	
	for(i=0; i<this->size; i++){
		if( list[i]->isInInterval(values) == TRUE)
			return TRUE;
	}
	
	return FALSE;
}

/**
	values [IN]
		This is a matrix N rows x 2 columns which N equals varCount
		
	@param outInterval
		This output parameter, it's a matrix N row and M columns which each row is for each continuous space for the expression
		It means that each row will hold a combined-interval for n-tule variables and M equal varCount * 2
*/
void getCompositeInterval(const void *interval, const float *values, int varCount, void *outDomainObj){
	CompositeCriteria *criteria = (CompositeCriteria*)interval;
	CompositeCriteria *outDomain = (CompositeCriteria *)outDomainObj;
	CombinedCriteria *listIn;
	int i;
	
	for(i=0; i<criteria->size; i++){
		listIn = newCombinedInterval();
		
		(criteria->list[i])->fgetInterval(criteria->list[i], values, varCount, listIn);
		if(listIn->size > 0 ){
			if(outDomain->size >= outDomain->loggedSize){
				outDomain->loggedSize += INCLEN;
				outDomain->list = (CombinedCriteria**)realloc(outDomain->list, sizeof(CombinedCriteria*) * outDomain->loggedSize);
			}
			outDomain->list[outDomain->size++] = listIn;
		}else{
			free(listIn);
		}
	}
}

/**
	@param domain
	result [OUT] MUST an level-2 NOT-NULL pointer
*/
void buildCompositeCriteria(const NMAST *domain, const char *vars, int varCount, OutBuiltCriteria *result){
	OutBuiltCriteria leftResult;
	OutBuiltCriteria rightResult;
	CombinedCriteria *outCombinedCriteria;
	Criteria *interval;
	int i, objTypeLeft, objTypeRight /*, type*/;
	
	if(domain == NULL){
		if(varCount == 1){
			result->cr = newCriteria(GT_LT, vars[0], -9999, 9999, TRUE, TRUE);
			return;
		}

		outCombinedCriteria = newCombinedInterval();
		outCombinedCriteria->loggedSize = varCount;
		outCombinedCriteria->size = 0;
		outCombinedCriteria->list = (Criteria**)malloc(sizeof(Criteria*) * outCombinedCriteria->loggedSize);
		for(i=0; i<varCount; i++) {
			interval = newCriteria(GT_LT, vars[i], -9999, 9999, TRUE, TRUE);
			outCombinedCriteria->list[(outCombinedCriteria->size)++] = interval;
		}
		result->cr = outCombinedCriteria;
		return;
	}
	
	leftResult.cr = NULL;
	rightResult.cr = NULL;
	switch(domain->type){
		case GT_LT:
		case GTE_LT:
		case GT_LTE:
		case GTE_LTE:
			result->cr = (void*)newCriteria(domain->type, domain->variable, domain->left->value, domain->right->value, FALSE, FALSE);
		break;
		
		case LT:
			result->cr = (void*)newCriteria(GT_LT, domain->left->variable, 99999, domain->right->value, TRUE, FALSE);
		break;
		
		case LTE:
			result->cr = (void*)newCriteria(GT_LTE, domain->left->variable, 99999, domain->right->value, TRUE, FALSE);
		break;
		
		case GT:
			result->cr = (void*)newCriteria(GT_LT, domain->left->variable, domain->right->value, 99999, FALSE, TRUE);
		break;
		
		case GTE:
			result->cr = (void*)newCriteria(GTE_LT, domain->left->variable, domain->right->value, 99999, FALSE, TRUE);
		break;
		
		case AND:
			if(domain->left == NULL || domain->right == NULL) //Do we need check NULL here?
				return;
			buildCompositeCriteria(domain->left, vars, varCount, &leftResult);
			buildCompositeCriteria(domain->right, vars, varCount, &rightResult);
			andTwoCriteria(leftResult.cr, rightResult.cr, result);
		break;
		
		case OR:
			if(domain->left == NULL || domain->right == NULL) //Do we need check NULL here?
				return;
			buildCompositeCriteria(domain->left, vars, varCount, &leftResult);
			buildCompositeCriteria(domain->right, vars, varCount, &rightResult);

			objTypeLeft = *((char*)(leftResult.cr));
			objTypeRight = *((char*)(rightResult.cr));
			
			if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
				orTwoSimpleCriteria((Criteria*)leftResult.cr, (Criteria*)rightResult.cr, result);
			} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
			} else if( objTypeLeft == SIMPLE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
			} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
			} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
			} else if( objTypeLeft == COMBINED_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
			} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == SIMPLE_CRITERIA ) {
			} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMBINED_CRITERIA ) {
			} else if( objTypeLeft == COMPOSITE_CRITERIA && objTypeRight == COMPOSITE_CRITERIA ) {
			
			}
		break;
		
		default:
		break;
	}
}