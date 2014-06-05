#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "funct.h"

TNode* d_product(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_quotient(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_sum_subtract(TNode *t, char optr, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_pow_exp(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_sqrt(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_sin(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_cos(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_tan(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_asin(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_acos(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
TNode* d_atan(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x);
int isContainVar(TNode *t, char x);

TNode *createTreeNode(){
	TNode *p = (TNode*)malloc(sizeof(TNode));
	p->valueType = TYPE_FLOATING_POINT;
	p->value = 0.0f;
	(p->frValue).numerator = 0;
	(p->frValue).denomerator = 1;
	p->sign = 1;
	p->parent = p->left = p->right;
	return p;
}

TNode * cloneTree(TNode *t, TNode *cloneParent){
	TNode *c;
	
	if(t == NULL)
		return NULL;
	
	c = (TNode*)malloc(sizeof(TNode));
	memcpy(c, t, sizeof(TNode));
	c->parent = cloneParent;
	c->left = cloneTree(t->left, c);
	c->right = cloneTree(t->right, c);
	return c;
}

void* derivative(void *p){
	DParam *dp = (DParam*)p;
	TNode *t = dp->t;
	TNode *u, *du, *v, *dv;
	pthread_t tdu, tdv;
	int id_du = -1, id_dv = -1;
	DParam pdu, pdv;
	
	if(t==NULL)
		return NULL;
	
	if(t->function == F_COE || t->function == F_CONSTAN){
		u = (TNode*)malloc(sizeof(TNode));
		u->function = F_COE;
		u->chr = COE;
		u->value = 0.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		return u;
	}
	
	if( t->function == F_VAR ){
		u = (TNode*)malloc(sizeof(TNode));
		u->function = F_COE;
		u->chr = COE;
		u->parent = NULL;
		u->left = u->right = NULL;
		
		if(dp->x == t->chr){
			u->value = 1.0;
			return u;
		}
		
		u->value = 0.0;
		return u;
	}
	
		
	dv = du = NULL;
	
	u = t->left;
	v = t->right;
	
	if(u!=NULL){
		pdu.t = t->left;
		pdu.x = dp->x;
		id_du = pthread_create(&tdu, NULL, derivative, (void*)(&pdu));
	}
	
	if(v != NULL){
		pdv.t = t->right;
		pdv.x = dp->x;
		id_dv = pthread_create(&tdv, NULL, derivative, (void*)(&pdv));
	}
	if(id_du == 0)
		pthread_join(tdu, (void**)&du);
	if(id_dv == 0)
		pthread_join(tdv, (void**)&dv);
	
	if(t->function == F_FUNCT){
		switch(t->chr){
			case SIN:
				return d_sin(t, u, du, v, dv, dp->x);
				
			case COS:
				return d_cos(t, u, du, v, dv, dp->x);
				
			case TAN:
				return d_tan(t, u, du, v, dv, dp->x);
				
			case ASIN:
				return d_asin(t, u, du, v, dv, dp->x);
				
			case ACOS:
				return d_acos(t, u, du, v, dv, dp->x);
				
			case ATAN:
				return d_atan(t, u, du, v, dv, dp->x);
				
			case SQRT:
				return d_sqrt(t, u, du, v, dv, dp->x);
		}
	}else if(t->function == F_OPT){
		switch(t->chr){
			case PLUS:
			case MINUS:
				return d_sum_subtract(t, t->chr, u, du, v, dv, dp->x);
			
			case MUL:
				return d_product(t, u, du, v, dv, dp->x);
			
			case DIV:
				return d_quotient(t, u, du, v, dv, dp->x);
				
			/* Pow is a function but act as an operator */	
			case POW:
				return d_pow_exp(t, u, du, v, dv, dp->x);
		}
	}
	
	/* WHERE du AND dv GO IF WE NOT TO USE THEM ????? */
	
	return NULL;
}
/* (u.v) = u'v + uv' */
TNode* d_product(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r = NULL;
	
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = PLUS;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_OPT;
	r->left->chr = MUL;
	r->left->parent = r;
	r->left->left = cloneTree(u, r->left);
	r->left->right = dv;
	if(dv != NULL)
		dv->parent = r->left;
		
	r->right = (TNode *)malloc(sizeof(TNode));
	r->right->function = F_OPT;
	r->right->chr = MUL;
	r->right->parent = r;
	r->right->left = du;
	if(du != NULL)
		du->parent = r->right;
	r->right->right = cloneTree(v, r->right);
	
	return r;
}

/*
 * (u/v)' = (u'v - uv')/v^2
 * 
 * */
TNode* d_quotient(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	
	r = (TNode*)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = DIV;
	r->value = 0;
	r->sign = 1;
	r->parent = NULL;
			
	r->left = (TNode*)malloc(sizeof(TNode));
	(r->left)->parent = r;
	(r->left)->function = F_OPT;
	(r->left)->chr = MINUS;
	(r->left)->value = 0;
	(r->left)->sign = 1;
	
	/* ========================================== */
	(r->left)->left = (TNode*)malloc(sizeof(TNode));
	(r->left)->left->function = F_OPT;
	(r->left)->left->chr = MUL;
	(r->left)->left->sign = 1;
	(r->left)->left->parent = r->right;
	((r->left)->left)->left = du;
	if(du != NULL)
		du->parent = (r->left)->left;
	((r->left)->left)->right = cloneTree(v, (r->left)->left);
	
	/* =================================================== */
	
	(r->left)->right = (TNode*)malloc(sizeof(TNode));
	(r->left)->right->function = F_OPT;
	(r->left)->right->chr = MUL;
	(r->left)->right->sign = 1;
	(r->left)->right->parent = r->right;
	((r->left)->right)->left = cloneTree(u, (r->left)->right);
	((r->left)->right)->right = dv;
	if(dv != NULL)
		dv->parent = (r->left)->right;
	
	/* ==================================================== */
			
	r->right = (TNode*)malloc(sizeof(TNode));
	(r->right)->parent = r;
	(r->right)->function = F_FUNCT;
	(r->right)->chr = POW;
	r->value = 0;
	r->sign = 1;
			
	(r->right)->left = cloneTree(v, r->right);
			
	(r->right)->right = (TNode*)malloc(sizeof(TNode));
	(r->right)->right->function = F_COE;
	(r->right)->right->chr = COE;
	(r->right)->right->value = 2;
	(r->right)->right->sign = 1;
	(r->right)->right->parent = r->right;
	((r->right)->right)->left = ((r->right)->right)->right = NULL;
	return r;
}

/*
 * (u +- v) = u' +- v'
 * */
TNode* d_sum_subtract(TNode *t, char optr, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = optr;
	r->value = 0.0;
	r->parent = NULL;
	
	r->left = du;
	if(du != NULL)
		du->parent = r;
		
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
		
	return r;
}

TNode* d_pow_exp(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	int isXLeft = isContainVar(u, x);
	int isXRight = isContainVar(v, x);
	
	/* power: (u^a)' = au^(a-1)*u' */
	if( isXLeft!=0 && isXRight==0 ){
		r = (TNode *)malloc(sizeof(TNode));
		r->function = F_OPT;
		r->chr = MUL;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (TNode *)malloc(sizeof(TNode));
		(r->left)->parent = r;
		(r->left)->function = F_OPT;
		(r->left)->chr = MUL;
		
		(r->left)->left = (TNode *)malloc(sizeof(TNode));
		((r->left)->left)->function = F_COE;
		((r->left)->left)->value = v->value;
		((r->left)->left)->sign = v->sign;
		((r->left)->left)->parent = (r->left);
		((r->left)->left)->left = ((r->left)->left)->right = NULL;
		
		/*printf(" Add Left Value 1/(v-1) \n");*/
		
		(r->left)->right = (TNode *)malloc(sizeof(TNode));
		((r->left)->right)->function = F_OPT;
		((r->left)->right)->value = 0.0;
		((r->left)->right)->sign = 1;
		((r->left)->right)->chr = POW;
		((r->left)->right)->parent = (r->left);
		
		/*printf(" Add Right POW \n");*/
		
		((r->left)->right)->left = cloneTree(u, ((r->left)->right));
		
		/*printf(" Add LEFT Copy U \n");*/
		
		((r->left)->right)->right = cloneTree(v, ((r->left)->right)); 
		
		(((r->left)->right)->right)->value = v->value - 1;
		/*printf(" Add Right v-1 \n");*/
		
		r->right = du;
		if(du!=NULL) /* <-- is in need? */
			du->parent = r;
		/*printf(" Add Right du \n");*/
		
		return r;
	}
	
	/* power: (a^v)' = ln(a)*a^v*v' */
	if( isXLeft==0 && isXRight!=0 ){
		r = (TNode *)malloc(sizeof(TNode));
		r->function = F_OPT;
		r->chr = MUL;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (TNode *)malloc(sizeof(TNode));
		(r->left)->parent = r;
		(r->left)->function = F_OPT;
		(r->left)->chr = MUL;
		
		(r->left)->left = (TNode *)malloc(sizeof(TNode));
		((r->left)->left)->function = F_FUNCT;
		((r->left)->left)->value = 0;
		((r->left)->left)->chr = LN;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		((r->left)->left)->left = NULL;
		((r->left)->left)->right = cloneTree(u, (r->left)->left);
		
		(r->left)->right = (TNode *)malloc(sizeof(TNode));
		((r->left)->right)->function = F_OPT;
		((r->left)->right)->value = 0;
		((r->left)->right)->chr = POW;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = r->left;
		((r->left)->right)->left = cloneTree(u, (r->left)->right);
		((r->left)->right)->right = cloneTree(v, (r->left)->right);
		
		/* ===================================================== */
		r->right = dv;
		if(dv != NULL)
			dv->parent = r;
			
		return r;
	}
	
	/* power: (u^v)' = (dv*ln(u) + v(du/u))*u^v */
	if( isXLeft!=0 && isXRight!=0 ){
		r = (TNode *)malloc(sizeof(TNode));
		r->function = F_OPT;
		r->chr = MUL;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (TNode *)malloc(sizeof(TNode));
		(r->left)->parent = r;
		(r->left)->function = F_OPT;
		(r->left)->chr = PLUS;
		
		(r->left)->left = (TNode *)malloc(sizeof(TNode));
		((r->left)->left)->function = F_OPT;
		((r->left)->left)->value = 0;
		((r->left)->left)->chr = MUL;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		
		((r->left)->left)->left = dv;
		if(dv != NULL)
			dv->parent = (r->left)->left;
			
		((r->left)->left)->right = (TNode *)malloc(sizeof(TNode));
		((r->left)->left)->right->function = F_FUNCT;
		((r->left)->left)->right->chr = LN;
		((r->left)->left)->right->left = NULL;
		((r->left)->left)->right->right = cloneTree(u, ((r->left)->left)->right);
		
		(r->left)->right = (TNode *)malloc(sizeof(TNode));
		((r->left)->right)->function = F_OPT;
		((r->left)->right)->value = 0;
		((r->left)->right)->chr = MUL;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = r->left;
		(r->left)->right->left = cloneTree(v, (r->left)->right);
		(r->left)->right->right = (TNode *)malloc(sizeof(TNode));
		(r->left)->right->right->function = F_OPT;
		(r->left)->right->right->chr = DIV;
		(r->left)->right->right->value = 0;
		(r->left)->right->right->left = du;
		if(du != NULL)
			du->parent = (r->left)->right->right;
		(r->left)->right->right->right = cloneTree(u, (r->left)->right->right);
		
		/* ===================================================== */
		r->right = (TNode *)malloc(sizeof(TNode));
		r->right->function = F_OPT;
		r->right->chr = POW;
		r->right->left = cloneTree(u, r->right);
		r->right->right = cloneTree(v, r->right);
		
		return r;
	}
	
	return NULL;
}

/* (sqrt(v))' = dv/(2*sqrt(v)) */
TNode* d_sqrt(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = DIV;
	r->sign = 1;
	r->parent = NULL;
		
	r->left = dv;
	if(dv != NULL)
		dv->parent = r;
		
	/*Right child: product operator */
	r->right = (TNode *)malloc(sizeof(TNode));
	r->right->function = F_OPT;
	r->right->value = 0;
	r->right->sign = 1;
	r->right->chr = MUL;
	r->right->parent = r;
		
	r->right->left = (TNode *)malloc(sizeof(TNode));
	r->right->left->function = F_COE;
	r->right->left->chr = COE;
	r->right->left->value = 2;
	r->right->left->sign = 1;
	r->right->left->valueType = TYPE_FLOATING_POINT;
	r->right->left->parent = r->right;
		
	r->right->right = cloneTree(t, r->right);
	return r;
}

/* (sin(v))' = cos(v)*dv */
TNode* d_sin(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	/* (cos(v))' = -sin(v)*dv */
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_FUNCT;
	r->left->chr = COS;
	r->left->sign = 1;
	r->left->parent = r;
	r->left->left = NULL;
	r->left->right = cloneTree(v, r);
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
		
	return r;
}

TNode* d_cos(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	/* (cos(v))' = -sin(v)*dv */
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_FUNCT;
	r->left->chr = SIN; /* <== negative here */
	r->left->sign = -1;
	r->left->parent = r;
	r->left->left = NULL;
	r->left->right = cloneTree(v, r);
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
		
	return r;
}

/* tan(v)' =  (sec(v)^2)*dv  */
TNode* d_tan(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_OPT;
	r->left->chr = POW;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (TNode *)malloc(sizeof(TNode));
	r->left->left->parent = r->left;
	r->left->left->function = F_FUNCT;
	r->left->left->chr = SEC;
	
	r->left->left->left = NULL;
	r->left->left->right = cloneTree(v, r->left->left);
	
	r->left->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->parent = r->left;
	r->left->right->function = F_COE;
	r->left->right->chr = COE;
	r->left->right->value = 2;
	r->left->right->sign = 1;
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
		
	return r;
}

/* arcsin(v)' = (1/sqrt(1-v^2))*dv */
TNode* d_asin(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_OPT;
	r->left->chr = DIV;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (TNode *)malloc(sizeof(TNode));
	r->left->left->function = F_COE;
	r->left->left->chr = COE;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* sqrt(...) */
	r->left->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->function = F_FUNCT;
	r->left->right->chr = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;
	
	r->left->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->function = F_OPT;
	r->left->right->right->chr = MINUS;
	r->left->right->right->parent = r->left->right;
	
	r->left->right->right->left = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->left->function = F_COE;
	r->left->right->right->left->chr = COE;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;
	
	r->left->right->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->right->function = F_OPT;
	r->left->right->right->right->chr = POW;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;
	
	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);
	
	r->left->right->right->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->right->right->function = F_COE;
	r->left->right->right->right->right->chr = COE;
	r->left->right->right->right->right->value = 2;
	r->left->right->right->right->right->sign = 1;
	r->left->right->right->right->right->parent = r->left->right->right->right;
	r->left->right->right->right->right->left = r->left->right->right->right->right->right = NULL;
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
	return r;
}

/* arccos(v)' = (-1/sqrt(1-v^2))*dv */
TNode* d_acos(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_OPT;
	r->left->chr = DIV;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (TNode *)malloc(sizeof(TNode));
	r->left->left->function = F_COE;
	r->left->left->chr = COE;
	r->left->left->value = -1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* sqrt(...) */
	r->left->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->function = F_FUNCT;
	r->left->right->chr = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;
	
	r->left->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->function = F_OPT;
	r->left->right->right->chr = MINUS;
	r->left->right->right->parent = r->left->right;
	
	r->left->right->right->left = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->left->function = F_COE;
	r->left->right->right->left->chr = COE;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;
	
	r->left->right->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->right->function = F_OPT;
	r->left->right->right->right->chr = POW;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;
	
	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);
	
	r->left->right->right->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->right->right->function = F_COE;
	r->left->right->right->right->right->chr = COE;
	r->left->right->right->right->right->value = 2;
	r->left->right->right->right->right->sign = 1;
	r->left->right->right->right->right->parent = r->left->right->right->right;
	r->left->right->right->right->right->left = r->left->right->right->right->right->right = NULL;
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
	return r;
}

/* arctan(v)' = (1/(v^2+1))*dv */
TNode* d_atan(TNode *t, TNode *u, TNode *du, TNode *v, TNode *dv, char x){
	TNode *r;
	r = (TNode *)malloc(sizeof(TNode));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (TNode *)malloc(sizeof(TNode));
	r->left->function = F_OPT;
	r->left->chr = DIV;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (TNode *)malloc(sizeof(TNode));
	r->left->left->function = F_COE;
	r->left->left->chr = COE;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* (v^2+1) */
	r->left->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->function = F_OPT;
	r->left->right->chr = PLUS;
	r->left->right->parent = r->left;
	
	r->left->right->left = (TNode *)malloc(sizeof(TNode));
	r->left->right->left->function = F_OPT;
	r->left->right->left->chr = POW;
	r->left->right->left->value = 0;
	r->left->right->left->sign = 1;
	r->left->right->left->parent = r->left->right;
	r->left->right->left->left = cloneTree(v, r->left->right->left);
	
	r->left->right->left->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->left->right->function = F_COE;
	r->left->right->left->right->chr = COE;
	r->left->right->left->right->value = 2;
	r->left->right->left->right->sign = 1;
	r->left->right->left->right->parent = r->left->right->left;
	r->left->right->left->right->left = r->left->right->left->right->right = NULL;
	
	r->left->right->right = (TNode *)malloc(sizeof(TNode));
	r->left->right->right->function = F_COE;
	r->left->right->right->chr = COE;
	r->left->right->right->value = 1;
	r->left->right->right->sign = 1;
	r->left->right->right->parent = r->left->right;
	r->left->right->right->left = r->left->right->right->right = NULL;
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
	return r;
}

int isContainVar(TNode *t, char x){
	
	if( (t==NULL) || (t->function==F_COE) || (t->function==F_CONSTAN) )
		return 0;
		
	if( t->function == F_VAR ){
		if(t->chr == x)
			return 1;
		return 0;
	}
	
	return (isContainVar(t->left,x) || isContainVar(t->right, x) );
}
