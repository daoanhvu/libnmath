#include "derivative.h"



NMAST *createTreeNode(){
	NMAST *p = (NMAST*)malloc(sizeof(NMAST));
	p->valueType = TYPE_FLOATING_POINT;
	p->value = 0.0f;
	(p->frValue).numerator = 0;
	(p->frValue).denomerator = 1;
	p->sign = 1;
	p->parent = p->left = p->right;
	return p;
}

NMAST * cloneTree(NMAST *t, NMAST *cloneParent){
	NMAST *c;
	
	if(t == NULL)
		return NULL;
	
	c = (NMAST*)malloc(sizeof(NMAST));
	memcpy(c, t, sizeof(NMAST));
	c->parent = cloneParent;
	c->left = cloneTree(t->left, c);
	c->right = cloneTree(t->right, c);
	return c;
}

unsigned int __stdcall derivative(void *p){
	DParam *dp = (DParam*)p;
	NMAST *t = dp->t;
	char x = dp->x;
	NMAST *u, *du, *v, *dv, *r;
	HANDLE tdu = 0, tdv = 0;
	DParam pdu, pdv;
	
	if(t==NULL){
		dp->return_value = NULL;
		return 0;
	}
	
	if(t->type == F_COE || t->type == F_CONSTAN ){
		u = (NMAST*)malloc(sizeof(NMAST));
		u->function = F_COE;
		u->chr = COE;
		u->value = 0.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		dp->return_value = u;
		return 0;
	}
	
	if(t->type == F_VAR){
		u = (NMAST*)malloc(sizeof(NMAST));
		u->function = F_COE;
		u->chr = COE;
		u->value = 1.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		dp->return_value = u;
		return 0;
	}
	
	dv = du = NULL;
	
	u = t->left;
	v = t->right;

	if( (u!=NULL) && ((u->function == F_FUNCT) || (u->function == F_OPT)) ){
		pdu.t = t->left;
		pdu.x = x;
		tdu = (HANDLE)_beginthreadex(NULL, 0, &derivative, (void*)&pdu, 0, NULL);
	}
	
	if( (v != NULL) && ((v->function == F_FUNCT) || (v->function == F_OPT)) ){
		pdv.t = t->right;
		pdv.x = x;
		tdv = (HANDLE)_beginthreadex(NULL, 0, &derivative, (void*)&pdv, 0, NULL);
	}

	if(tdu != 0){
		WaitForSingleObject(tdu, INFINITE);
		du = pdu.return_value;
		CloseHandle(tdu);
	}
	if(tdv != 0){
		WaitForSingleObject(tdv, INFINITE);
		dv = pdv.return_value;
		CloseHandle(tdv);
	}
	
	if(t->type == F_FUNCT){
		switch(t->chr){
			case SIN:
				dp->return_value = d_sin(t, u, du, v, dv, x);
				return 0;
				
			case COS:
				dp->return_value = d_cos(t, u, du, v, dv, x);
				return 0;
				
			case SQRT:
				dp->return_value = d_sqrt(t, u, du, v, dv, x);
				return 0;
		}
	}else if(t->type == F_OPT){
		switch(t->chr){
			case PLUS:
			case MINUS:
				dp->return_value = d_sum_subtract(t, t->chr, u, du, v, dv, x);
				return 0;
			
			case MUL:
				dp->return_value = d_product(t, u, du, v, dv, x);
				return 0;
				
			case DIV:
				dp->return_value = d_quotient(t, u, du, v, dv, x);
				return 0;
				
			case POW:
				dp->return_value = d_pow_exp(t, u, du, v, dv, x);
				return 0;
		}
	}
	dp->return_value = NULL;
	return 0;
}

/* (u.v) = u'v + uv' */
NMAST* d_product(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r = NULL;
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = PLUS;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->function = F_OPT;
	r->left->chr = MUL;
	r->left->parent = r;
	r->left->left = cloneTree(u, r->left);
	r->left->right = dv;
	if(dv != NULL)
		dv->parent = r->left;
		
	r->right = (NMAST *)malloc(sizeof(NMAST));
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
 * */
NMAST* d_quotient(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	
	r = (NMAST*)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = DIV;
	r->value = 0;
	r->sign = 1;
	r->parent = NULL;
			
	r->left = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->parent = r;
	(r->left)->function = F_OPT;
	(r->left)->chr = MINUS;
	(r->left)->value = 0;
	(r->left)->sign = 1;
	
	/* ========================================== */
	(r->left)->left = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->left->function = F_OPT;
	(r->left)->left->chr = MUL;
	(r->left)->left->sign = 1;
	(r->left)->left->parent = r->right;
	((r->left)->left)->left = du;
	if(du != NULL)
		du->parent = (r->left)->left;
	((r->left)->left)->right = cloneTree(v, (r->left)->left);
	
	/* =================================================== */
	
	(r->left)->right = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->right->function = F_OPT;
	(r->left)->right->chr = MUL;
	(r->left)->right->sign = 1;
	(r->left)->right->parent = r->right;
	((r->left)->right)->left = cloneTree(u, (r->left)->right);
	((r->left)->right)->right = dv;
	if(dv != NULL)
		dv->parent = (r->left)->right;
	
	/* ==================================================== */
			
	r->right = (NMAST*)malloc(sizeof(NMAST));
	(r->right)->parent = r;
	(r->right)->function = F_FUNCT;
	(r->right)->chr = POW;
	r->value = 0;
	r->sign = 1;
			
	(r->right)->left = cloneTree(v, r->right);
			
	(r->right)->right = (NMAST*)malloc(sizeof(NMAST));
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
NMAST* d_sum_subtract(NMAST *t, char optr, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	
	r = (NMAST *)malloc(sizeof(NMAST));
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


NMAST* d_pow_exp(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	int isXLeft = isContainVar(u, x);
	int isXRight = isContainVar(v, x);
	
	/* power: (u^a)' = au^(a-1)*u' */
	if( isXLeft!=0 && isXRight==0 ){
		r = (NMAST *)malloc(sizeof(NMAST));
		r->function = F_OPT;
		r->chr = MUL;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->function = F_OPT;
		(r->left)->chr = MUL;
		
		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->function = F_COE;
		((r->left)->left)->value = v->value;
		((r->left)->left)->sign = v->sign;
		((r->left)->left)->parent = (r->left);
		((r->left)->left)->left = ((r->left)->left)->right = NULL;
		
		/*printf(" Add Left Value 1/(v-1) \n");*/
		
		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
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
		r = (NMAST *)malloc(sizeof(NMAST));
		r->function = F_OPT;
		r->chr = MUL;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->function = F_OPT;
		(r->left)->chr = MUL;
		
		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->function = F_FUNCT;
		((r->left)->left)->value = 0;
		((r->left)->left)->chr = LN;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		((r->left)->left)->left = NULL;
		((r->left)->left)->right = cloneTree(u, (r->left)->left);
		
		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
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
		r = (NMAST *)malloc(sizeof(NMAST));
		r->function = F_OPT;
		r->chr = MUL;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->function = F_OPT;
		(r->left)->chr = PLUS;
		
		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->function = F_OPT;
		((r->left)->left)->value = 0;
		((r->left)->left)->chr = MUL;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		
		((r->left)->left)->left = dv;
		if(dv != NULL)
			dv->parent = (r->left)->left;
			
		((r->left)->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->right->function = F_FUNCT;
		((r->left)->left)->right->chr = LN;
		((r->left)->left)->right->left = NULL;
		((r->left)->left)->right->right = cloneTree(u, ((r->left)->left)->right);
		
		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->function = F_OPT;
		((r->left)->right)->value = 0;
		((r->left)->right)->chr = MUL;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = r->left;
		(r->left)->right->left = cloneTree(v, (r->left)->right);
		(r->left)->right->right = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->right->right->function = F_OPT;
		(r->left)->right->right->chr = DIV;
		(r->left)->right->right->value = 0;
		(r->left)->right->right->left = du;
		if(du != NULL)
			du->parent = (r->left)->right->right;
		(r->left)->right->right->right = cloneTree(u, (r->left)->right->right);
		
		/* ===================================================== */
		r->right = (NMAST *)malloc(sizeof(NMAST));
		r->right->function = F_OPT;
		r->right->chr = POW;
		r->right->left = cloneTree(u, r->right);
		r->right->right = cloneTree(v, r->right);
		
		return r;
	}
	
	return NULL;
}


/* (sqrt(v))' = dv/(2*sqrt(v)) */
NMAST* d_sqrt(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = DIV;
	r->sign = 1;
	r->parent = NULL;
		
	r->left = dv;
	if(dv != NULL)
		dv->parent = r;
		
	/*Right child: product operator */
	r->right = (NMAST *)malloc(sizeof(NMAST));
	r->right->function = F_OPT;
	r->right->value = 0;
	r->right->sign = 1;
	r->right->chr = MUL;
	r->right->parent = r;
		
	r->right->left = (NMAST *)malloc(sizeof(NMAST));
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
NMAST* d_sin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	/* (cos(v))' = -sin(v)*dv */
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
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

/* (cos(v))' = -sin(v)dv */
NMAST* d_cos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	/* (cos(v))' = -sin(v)*dv */
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
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
NMAST* d_tan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->function = F_OPT;
	r->left->chr = POW;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->parent = r->left;
	r->left->left->function = F_FUNCT;
	r->left->left->chr = SEC;
	
	r->left->left->left = NULL;
	r->left->left->right = cloneTree(v, r->left->left);
	
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
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
NMAST* d_asin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->function = F_OPT;
	r->left->chr = DIV;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->function = F_COE;
	r->left->left->chr = COE;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* sqrt(...) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->function = F_FUNCT;
	r->left->right->chr = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;
	
	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->function = F_OPT;
	r->left->right->right->chr = MINUS;
	r->left->right->right->parent = r->left->right;
	
	r->left->right->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->left->function = F_COE;
	r->left->right->right->left->chr = COE;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;
	
	r->left->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->function = F_OPT;
	r->left->right->right->right->chr = POW;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;
	
	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);
	
	r->left->right->right->right->right = (NMAST *)malloc(sizeof(NMAST));
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
NMAST* d_acos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->function = F_OPT;
	r->left->chr = DIV;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->function = F_COE;
	r->left->left->chr = COE;
	r->left->left->value = -1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* sqrt(...) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->function = F_FUNCT;
	r->left->right->chr = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;
	
	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->function = F_OPT;
	r->left->right->right->chr = MINUS;
	r->left->right->right->parent = r->left->right;
	
	r->left->right->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->left->function = F_COE;
	r->left->right->right->left->chr = COE;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;
	
	r->left->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->function = F_OPT;
	r->left->right->right->right->chr = POW;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;
	
	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);
	
	r->left->right->right->right->right = (NMAST *)malloc(sizeof(NMAST));
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
NMAST* d_atan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->function = F_OPT;
	r->chr = MUL;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->function = F_OPT;
	r->left->chr = DIV;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->function = F_COE;
	r->left->left->chr = COE;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* (v^2+1) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->function = F_OPT;
	r->left->right->chr = PLUS;
	r->left->right->parent = r->left;
	
	r->left->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->function = F_OPT;
	r->left->right->left->chr = POW;
	r->left->right->left->value = 0;
	r->left->right->left->sign = 1;
	r->left->right->left->parent = r->left->right;
	r->left->right->left->left = cloneTree(v, r->left->right->left);
	
	r->left->right->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->right->function = F_COE;
	r->left->right->left->right->chr = COE;
	r->left->right->left->right->value = 2;
	r->left->right->left->right->sign = 1;
	r->left->right->left->right->parent = r->left->right->left;
	r->left->right->left->right->left = r->left->right->left->right->right = NULL;
	
	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
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

int isContainVar(NMAST *t, char x){
	
	if( (t==NULL) || (t->type==F_COE) || (t->type==F_CONSTAN) )
		return 0;
		
	if( t->type == F_VAR ){
		if(t->chr == x)
			return 1;
		return 0;
	}
	
	return (isContainVar(t->left,x) || isContainVar(t->right, x) );
}
