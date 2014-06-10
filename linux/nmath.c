#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef unix
#include <pthread.h>
#endif

#include "nmath.h"

NMAST* d_product(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_quotient(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_sum_subtract(NMAST *t, int type, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_pow_exp(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_sqrt(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_sin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_cos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_tan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_cotan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_asin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_acos(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);
NMAST* d_atan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x);

/*
	Check if a tree contains variable x
	@param t the tree
	@param x variable to check if existed
*/
int isContainVar(NMAST *t, char x);

int isInArray(char *vars, char c){
	if(c == vars[0])
		return 0;

	if(c == vars[1])
		return 1;

	if(c == vars[2])
		return 2;

	return -1;
}

void initFunct(Function *f){
	f->str = NULL;
	f->len = 0;

	f->variable[0] = 0;
	f->variable[1] = 0;
	f->variable[2] = 0;
	f->valLen = 0;

	f->prefix = NULL;
	f->prefixLen = 0;
	f->prefixAllocLen = 0;

	f->variableNode = NULL;
	f->numVarNode = 0;
}

void releaseFunct(Function *f){
	int i;
	NMAST **temp;
	if(f==NULL) return;

	if(f->str != NULL)
		free(f->str);
	f->len = 0;

	for(i=0;i<f->prefixLen;i++){
		clearTree(&(f->prefix[i]));
	}
	f->prefixLen = 0;
	f->prefixAllocLen = 0;
	temp = f->prefix;
	free(temp);
	f->prefix = NULL;

	if(f->numVarNode > 0){
		temp = f->variableNode;
		free(temp);
		f->variableNode = NULL;
	}
}

void resetFunction(Function *f, const char *str, const char *vars, int varCount, int *error){
	int i=0, l=0, flagFirst = 0;
	char buf[1024];
	
	(*error) = 0;
	while(str[i]!='\0'){
		if(str[i] != ' '){
			if(!flagFirst){
				flagFirst = 1;
				switch(str[i]){
					case '+':
						i++;
					break;
					
					case '^':
					case '*':
					case '/':
						(*error) = i+1;
						return;
					break;
					
					default:
						buf[l] = str[i];
						l++;
					break;
				}
			}else{
				buf[l] = str[i];
				l++;
			}
		}
		i++;
	}

	if(l != f->len){
		f->str = (char*)realloc(f->str, l);
		f->len = l;
	}
	memcpy(f->str, buf, l);

	for(i=0; i<varCount; i++)
		f->variable[i] = vars[i];
	f->valLen = varCount;

	for(i=0; i<f->prefixLen; i++)
		clearTree(&(f->prefix[i]));
	f->prefixLen = 0;
	(*error) = 0;
}

void* reduce_t(void *param){
	RParam *dp = (RParam *)param;
	NMAST *p;
	RParam this_param_left;
	RParam this_param_right;
	pthread_t thrLeft, thrRight;
	int idThrLeft=-1, idThrRight = -1;
	this_param_left.error = this_param_right.error = 0;
	
	/* If the tree is NULL */
	if((dp->t)==NULL){
		return &(dp->error);
	}

	/*
		If this node is has left child, and the left child is an operator or a function then we
		create a thread to reduce the left child.
	*/
	if( ((dp->t)->left) != NULL && isFunctionOROperator(((dp->t)->left)->type) ){
		this_param_left.t = (dp->t)->left;
		idThrLeft = pthread_create(&thrLeft, NULL, reduce_t, (void*)(&this_param_left));
	}
		
	/*
		If this node is has right child, and the right child is an operator or a function then we
		create a thread to reduce the right child.
	*/
	if( ((dp->t)->right) != NULL && isFunctionOROperator(((dp->t)->right)->type) ){
		this_param_right.t = (dp->t)->right;
		idThrRight = pthread_create(&thrRight, NULL, reduce_t, (void*)(&this_param_right));
	}
	if(idThrLeft == 0)
		pthread_join(thrLeft, NULL);
	if(idThrRight == 0)
		pthread_join(thrRight, NULL);

	if(this_param_left.error != 0){
		dp->error = this_param_left.error;
		return &(dp->error);
	}
	
	if(this_param_right.error != 0){
		dp->error = this_param_right.error;
		return &(dp->error);
	}
	/*************************************************************************************/
	
	/*
		We don't reduce a node if it's a variable, a number, PI, E
	*/
	if(((dp->t)->type == VARIABLE) || isConstant((dp->t)->type))
		return &(dp->error);
		
	//if this node is an operator
	if( ((dp->t)->type == PLUS) || ((dp->t)->type == MINUS) || ((dp->t)->type == MULTIPLY)
					|| ((dp->t)->type == DIVIDE) || ((dp->t)->type == POWER) ){	
		if(((dp->t)->left!=NULL) && ((dp->t)->right!=NULL)){
			/* take care of special cases */
			if((dp->t)->type == PLUS){
				/* 0 + something */
				if( (  ((dp->t)->left)->type == NUMBER ) && ((dp->t)->left)->value == 0.0 ){
				
					//remove the left child (value 0)
					p = (dp->t)->left;
					free(p);
					(dp->t)->left = NULL;
					
					//move right child to its parent
					if((dp->t)->right != NULL){
						p = (dp->t)->right;
						
						(dp->t)->type = p->type;
						//(dp->t)->variable = p->variable;
						(dp->t)->value = p->value;
						(dp->t)->valueType = p->valueType;
						(dp->t)->frValue = p->frValue;
						//(dp->t)->priority = p->priority;
						(dp->t)->sign = p->sign;
						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						
						/* Now release p */
						free(p);
						p = NULL;
						return &(dp->error);
					}
				}
				
				/* something + 0 */
				if( ( ((dp->t)->right)->type == NUMBER) && (((dp->t)->right)->value == 0.0) ){
				
					//remove the right child (value 0)
					p = (dp->t)->right;
					free(p);
					(dp->t)->right = NULL;
					
					if((dp->t)->left != NULL){
						p = (dp->t)->left;
						
						(dp->t)->type = p->type;
						//(dp->t)->variable = p->variable;
						(dp->t)->value = p->value;
						(dp->t)->valueType = p->valueType;
						(dp->t)->frValue = p->frValue;
						//(dp->t)->priority = p->priority;
						(dp->t)->sign = p->sign;
						
						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						
						/* Now release p */
						free(p);
						p = NULL;
						return &(dp->error);
					}
				}
				
				//Left and right child of this node are null, 
				// I'm not sure that this piece of code can be reached
				if( ((dp->t)->left == NULL) || ((dp->t)->right == NULL) )
					return &(dp->error);
			}
			
			if((dp->t)->type == MULTIPLY){
				/* (0 * something) OR (something * 0) */
				if( (((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 0) ||
						(((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 0) ){
					
					clearTree(&((dp->t)->left));
					clearTree(&((dp->t)->right));
					
					(dp->t)->type = NUMBER;
					(dp->t)->value = 0;
					(dp->t)->sign = 1;
					
					/* MUST return here */
					return &(dp->error);
				}
				
				/* (1 * something) */
				if( (((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 1) ){
					
					clearTree(&((dp->t)->left));
					p = (dp->t)->right;

					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					//(dp->t)->variable = p->variable;
					(dp->t)->frValue = p->frValue;
					//(dp->t)->priority = p->priority;
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;

					free(p);
					
					return &(dp->error);
				}

				/* (something * 1) */
				if( (((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 1) ){
					clearTree(&((dp->t)->right));
					p = (dp->t)->left;
					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					//(dp->t)->variable = p->variable;
					(dp->t)->frValue = p->frValue;
					//(dp->t)->priority = p->priority;
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;
					free(p);
					return &(dp->error);
				}
			}
						
			if((dp->t)->type == POWER){
				if(((dp->t)->right)->type == NUMBER){
					if( ((dp->t)->right)->value == 0.0 ){
						// something ^ 0 
						clearTree(&((dp->t)->left));
						clearTree(&((dp->t)->right));
						
						(dp->t)->type = NUMBER;
						//(dp->t)->variable = 0;
						(dp->t)->valueType = TYPE_FLOATING_POINT;
						(dp->t)->value = 1.0;
						((dp->t)->frValue).numerator = 1;
						((dp->t)->frValue).denomerator = 1;
						//(dp->t)->priority = COE_VAL_PRIORITY;
						(dp->t)->sign = 1;
						(dp->t)->left = (dp->t)->right = NULL;
						return &(dp->error);
					}
					
					if( ((dp->t)->right)->value == 1.0 ){
						// something ^ 1 
						p = (dp->t)->right;
						free(p);
						
						p = (dp->t)->left;
						
						(dp->t)->type = p->type;
						//(dp->t)->variable = p->variable;
						(dp->t)->valueType = p->valueType;
						(dp->t)->value = p->value;
						(dp->t)->frValue = p->frValue;
						//(dp->t)->priority = p->priority;
						(dp->t)->sign = p->sign;
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						
						free(p);
						
						return &(dp->error);
					}
				}
				
				// 0 ^ something
				if((((dp->t)->left)->type == NUMBER) && (((dp->t)->left)->value == 0.0 )){
				
					p = (dp->t)->right;
					free(p);
						
					p = (dp->t)->left;
					free(p);
						
					(dp->t)->type = NUMBER;
					(dp->t)->valueType = TYPE_FLOATING_POINT;
					(dp->t)->value = 0.0;
					((dp->t)->frValue).numerator = 0;
					((dp->t)->frValue).denomerator = 1;
					(dp->t)->sign = p->sign;
					(dp->t)->left = (dp->t)->right = NULL;
					 
					return &(dp->error);
				}
				
				// number ^ number, this is a trivial case
				if( ((dp->t)->left!=NULL) && ((dp->t)->right!=NULL) && isConstant(((dp->t)->left)->type) 
						&& isConstant(((dp->t)->right)->type) ){
					// printf("Process 2-opt function %d\n", (t->right)->type);
					(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
					
					if(dp->error != 0)
						return &(dp->error);
						
					(dp->t)->type = NUMBER;
					p = (dp->t)->left;
					free(p);
					p = (dp->t)->right;
					free(p);
					(dp->t)->left = (dp->t)->right = NULL;
					return &(dp->error);
				}
			} // END OPERATOR POWER
			
			// trivial cases
			if( isConstant(((dp->t)->left)->type) && isConstant(((dp->t)->right)->type) ){
				/*printf("doCalculate %c\n", t->type);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				if(dp->error != 0)
					return &(dp->error);
				(dp->t)->type = NUMBER;
				p = (dp->t)->left;
				free(p);
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
				return &(dp->error);
			}
		}
	}
	
	/*
		Right now, I dont take case specicial case for functions like SIN(PI), LN(E)
	*/
	switch( (dp->t)->type ){
		case SIN:
		case COS:
		case TAN:
		case COTAN:
		case ASIN:
		case ACOS:
		case ATAN:
		case LN:
		case SQRT:
			/*printf("Process function %d\n", t->type);*/
			if( (dp->t)->right != NULL){
				if(((dp->t)->right)->type==NUMBER){
					(dp->t)->value = doCalculate(0, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
					if(dp->error != 0)
						return &(dp->error);
					(dp->t)->type = NUMBER;
					if((dp->t)->left != NULL){
						p = (dp->t)->left;
						free(p);
					}
					p = (dp->t)->right;
					free(p);
					(dp->t)->left = (dp->t)->right = NULL;
					return &(dp->error);
				}
			}
			/*printf("End Process function %d\n", t->type);*/
		break;
			
		/* Power is an operator
		case POWER:
		break;
		*/
			
		case LOG:
			if( ((dp->t)->left!=NULL) && ((dp->t)->right!=NULL) && isConstant(((dp->t)->left)->type) 
					&& isConstant(((dp->t)->right)->type) ){
				/*printf("Process 2-opt function %d\n", (t->right)->type);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				
				if(dp->error != 0)
					return &(dp->error);
					
				(dp->t)->type = NUMBER;
				p = (dp->t)->left;
				free(p);
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
				return &(dp->error);
			}
		break;
	}
	return &(dp->error);
}

int reduce(Function *f, int *error){
	RParam dp;
	dp.t = *(f->prefix);
	dp.error = 0;
	reduce_t(&dp);
	return 0;
}

void* calc_t(void *param){
	RParam *dp = (RParam *)param;
	NMAST *t = dp->t;
	RParam this_param_left;
	RParam this_param_right;
	pthread_t thrLeft, thrRight;
	int idThrLeft=-1, idThrRight = -1;
	int var_index = -1;

	this_param_left.error = this_param_right.error = 0;
	this_param_left.variables = this_param_right.variables = dp->variables;
	this_param_left.values = this_param_right.values = dp->values;
	
	/* If the input tree is NULL, we do nothing */
	if(t==NULL){
		return NULL;
	}

	if(t->type == VARIABLE){
		var_index = isInArray(dp->variables, t->variable);
		dp->retv = dp->values[var_index];
		return &(dp->error);
	}
		
	if( (t->type == NUMBER) || (t->type == PI_TYPE) ||(t->type == E_TYPE) ){
		dp->retv = t->value;
		return &(dp->error);
	}

	this_param_left.t = t->left;
	idThrLeft = pthread_create(&thrLeft, NULL, calc_t, (void*)&this_param_left);
	this_param_right.t = t->right;
	idThrRight = pthread_create(&thrRight, NULL, calc_t, (void*)&this_param_right);
	
	if(idThrLeft == 0){
		pthread_join(thrLeft, NULL);
	}
	
	if(idThrRight == 0){
		pthread_join(thrRight, NULL);
	}
	/*******************************************************************************/

	/* Actually, we don't need to check error here b'cause the reduce phase does that
	if(this_param_left.error != 0){
		dp->error = this_param_left.error;
		return dp->error;
	}
	
	if(this_param_right.error != 0){
		dp->error = this_param_right.error;
		return dp->error;
	}*/
		
	dp->retv = doCalculate(this_param_left.retv, this_param_right.retv, t->type, &(dp->error));
	return &(dp->error);
}

double calc(Function *f, double *values, int numOfValue, int *error){
	RParam rp;
	//int i;

	rp.error = 0;
	rp.t = *(f->prefix);
	rp.values = values;
	rp.variables = f->variable;

	//replace variable by value
	//for(i=0; i<f->numVarNode; i++){
	//}

	calc_t((void*)&rp);
	return rp.retv;
}

//************  Derivative & Calculus Section ****************************************************

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

void* derivative(void *p){
	DParam *dp = (DParam*)p;
	NMAST *t = dp->t;
	char x = dp->x;
	NMAST *u, *du, *v, *dv;
	pthread_t tdu, tdv;
	int id_du = -1, id_dv = -1;
	DParam pdu, pdv;
	
	if(t==NULL){
		return NULL;
	}
	
	if(t->type == NUMBER || t->type == PI_TYPE|| t->type == E_TYPE ){
		u = (NMAST*)malloc(sizeof(NMAST));
		u->type = NUMBER;
		u->value = 0.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		return u;
	}
	
	/*
		IMPORTANT:
			In case of multi-variable function, we need to tell which variable that we are 
			getting derivative of
	*/
	if(t->type == VARIABLE){
		u = (NMAST*)malloc(sizeof(NMAST));
		u->type = NUMBER;
		u->value = 1.0;
		u->parent = NULL;
		u->left = u->right = NULL;
		
		if(dp->x == t->variable){
			u->value = 1.0;
			return u;
		}
		
		u->value = 0.0;
		return u;
	}

	dv = du = NULL;
	
	u = t->left;
	v = t->right;

	if(u!=NULL) {
		pdu.t = t->left;
		pdu.x = x;
		id_du = pthread_create(&tdu, NULL, derivative, (void*)(&pdu));
	}
	
	if(v != NULL){
		pdv.t = t->right;
		pdv.x = x;
		id_dv = pthread_create(&tdv, NULL, derivative, (void*)(&pdv));
	}
	if(id_du == 0)
		pthread_join(tdu, (void**)&du);
	if(id_dv == 0)
		pthread_join(tdv, (void**)&dv);
		
	/****************************************************************/
	// 2.0 get done here	
	switch(t->type){
		case SIN:
			return d_sin(t, u, du, v, dv, x);
			
		case COS:
			return d_cos(t, u, du, v, dv, x);

		case TAN:
			return d_tan(t, u, du, v, dv, dp->x);

		case COTAN:
			return d_cotan(t, u, du, v, dv, dp->x);
				
		case ASIN:
			return d_asin(t, u, du, v, dv, dp->x);
				
		case ACOS:
			return d_acos(t, u, du, v, dv, dp->x);
				
		case ATAN:
			return d_atan(t, u, du, v, dv, dp->x);
				
		case SQRT:
			return d_sqrt(t, u, du, v, dv, x);

		case PLUS:
		case MINUS:
			return d_sum_subtract(t, t->type, u, du, v, dv, x);
			
		case MULTIPLY:
			return d_product(t, u, du, v, dv, x);
				
		case DIVIDE:
			return d_quotient(t, u, du, v, dv, x);
				
		case POWER:
			return d_pow_exp(t, u, du, v, dv, x);
	}
	
	/* WHERE du AND dv GO IF WE NOT TO USE THEM ????? */
	return NULL;
}

/* (u.v) = u'v + uv' */
NMAST* d_product(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r = NULL;
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = PLUS;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = MULTIPLY;
	r->left->parent = r;
	r->left->left = cloneTree(u, r->left);
	r->left->right = dv;
	if(dv != NULL)
		dv->parent = r->left;
		
	r->right = (NMAST *)malloc(sizeof(NMAST));
	r->right->type = MULTIPLY;
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
	r->type = DIVIDE;
	r->value = 0;
	r->sign = 1;
	r->parent = NULL;
			
	r->left = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->parent = r;
	(r->left)->type = MINUS;
	(r->left)->value = 0;
	(r->left)->sign = 1;
	
	/* ========================================== */
	(r->left)->left = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->left->type = MULTIPLY;
	(r->left)->left->sign = 1;
	(r->left)->left->parent = r->right;
	((r->left)->left)->left = du;
	if(du != NULL)
		du->parent = (r->left)->left;
	((r->left)->left)->right = cloneTree(v, (r->left)->left);
	
	/* =================================================== */
	
	(r->left)->right = (NMAST*)malloc(sizeof(NMAST));
	(r->left)->right->type = MULTIPLY;
	(r->left)->right->sign = 1;
	(r->left)->right->parent = r->right;
	((r->left)->right)->left = cloneTree(u, (r->left)->right);
	((r->left)->right)->right = dv;
	if(dv != NULL)
		dv->parent = (r->left)->right;
	
	/* ==================================================== */
			
	r->right = (NMAST*)malloc(sizeof(NMAST));
	(r->right)->parent = r;
	(r->right)->type = POWER;
	r->value = 0;
	r->sign = 1;
			
	(r->right)->left = cloneTree(v, r->right);
			
	(r->right)->right = (NMAST*)malloc(sizeof(NMAST));
	(r->right)->right->type = NUMBER;
	(r->right)->right->value = 2;
	(r->right)->right->sign = 1;
	(r->right)->right->parent = r->right;
	((r->right)->right)->left = ((r->right)->right)->right = NULL;
	return r;
} //2.0 got here

/*
 * (u +- v) = u' +- v'
 * */
NMAST* d_sum_subtract(NMAST *t, int type, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = type;
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
		r->type = MULTIPLY;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->type = MULTIPLY;
		
		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->type = NUMBER;
		((r->left)->left)->value = v->value;
		((r->left)->left)->sign = v->sign;
		((r->left)->left)->parent = (r->left);
		((r->left)->left)->left = ((r->left)->left)->right = NULL;
		
		/*printf(" Add Left Value 1/(v-1) \n");*/
		
		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->type = POWER;
		((r->left)->right)->value = 0.0;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = (r->left);
		
		/*printf(" Add Right POWER \n");*/
		
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
		r->type = MULTIPLY;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->type = MULTIPLY;
		
		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->type = LN;
		((r->left)->left)->value = 0;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		((r->left)->left)->left = NULL;
		((r->left)->left)->right = cloneTree(u, (r->left)->left);
		
		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->type = POWER;
		((r->left)->right)->value = 0;
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
		r->type = MULTIPLY;
		r->value = 0.0;
		r->parent = NULL;
		
		/* ===================================================== */
		r->left = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->parent = r;
		(r->left)->type = PLUS;
		
		(r->left)->left = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->type = MULTIPLY;
		((r->left)->left)->value = 0;
		((r->left)->left)->sign = 1;
		((r->left)->left)->parent = r->left;
		
		((r->left)->left)->left = dv;
		if(dv != NULL)
			dv->parent = (r->left)->left;
			
		((r->left)->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->left)->right->type = LN;
		((r->left)->left)->right->left = NULL;
		((r->left)->left)->right->right = cloneTree(u, ((r->left)->left)->right);
		
		(r->left)->right = (NMAST *)malloc(sizeof(NMAST));
		((r->left)->right)->type = MULTIPLY;
		((r->left)->right)->value = 0;
		((r->left)->right)->sign = 1;
		((r->left)->right)->parent = r->left;
		(r->left)->right->left = cloneTree(v, (r->left)->right);
		(r->left)->right->right = (NMAST *)malloc(sizeof(NMAST));
		(r->left)->right->right->type = DIVIDE;
		(r->left)->right->right->value = 0;
		(r->left)->right->right->left = du;
		if(du != NULL)
			du->parent = (r->left)->right->right;
		(r->left)->right->right->right = cloneTree(u, (r->left)->right->right);
		
		/* ===================================================== */
		r->right = (NMAST *)malloc(sizeof(NMAST));
		r->right->type = POWER;
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
	r->type = DIVIDE;
	r->sign = 1;
	r->parent = NULL;
		
	r->left = dv;
	if(dv != NULL)
		dv->parent = r;
		
	/*Right child: product operator */
	r->right = (NMAST *)malloc(sizeof(NMAST));
	r->right->value = 0;
	r->right->sign = 1;
	r->right->type = MULTIPLY;
	r->right->parent = r;
		
	r->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->right->left->type = NUMBER;
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
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = COS;
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
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = SIN; /* <== negative here */
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
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = POWER;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->parent = r->left;
	r->left->left->type = SEC;
	
	r->left->left->left = NULL;
	r->left->left->right = cloneTree(v, r->left->left);
	
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->parent = r->left;
	r->left->right->type = NUMBER;
	r->left->right->value = 2;
	r->left->right->sign = 1;
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
		
	return r;
}

/* cotan(v)' = -(1 -sqrt(cotan(v))) * dv  */
NMAST* d_cotan(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;

	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = PLUS;
	r->left->value = 1.0;
	r->left->valueType = TYPE_FLOATING_POINT;
	r->left->frValue.numerator = 1;
	r->left->frValue.denomerator = 1;
	r->left->sign = -1;
	r->left->parent = r;

	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = 1.0;
	r->left->left->valueType = TYPE_FLOATING_POINT;
	r->left->left->frValue.numerator = 1;
	r->left->left->frValue.denomerator = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;

	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = SQRT;
	r->left->right->value = 1.0;
	r->left->right->valueType = TYPE_FLOATING_POINT;
	r->left->right->frValue.numerator = 1;
	r->left->right->frValue.denomerator = 1;
	r->left->right->sign = 1;
	r->left->right->parent = r->left;

	r->left->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->type = COTAN;
	r->left->right->left->value = 1.0;
	r->left->right->left->valueType = TYPE_FLOATING_POINT;
	r->left->right->left->frValue.numerator = 1;
	r->left->right->left->frValue.denomerator = 1;
	r->left->right->left->sign = 1;
	r->left->right->left->parent = r->left->right;

	r->left->right->left->left = cloneTree(v, r->left->right->left);

	r->right = dv;
	if(dv != NULL)
		dv->parent = r;

	return r;
}

/* arcsin(v)' = (1/sqrt(1-v^2))*dv */
NMAST* d_asin(NMAST *t, NMAST *u, NMAST *du, NMAST *v, NMAST *dv, char x){
	NMAST *r;
	r = (NMAST *)malloc(sizeof(NMAST));
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = DIVIDE;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* sqrt(...) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;
	
	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->type = MINUS;
	r->left->right->right->parent = r->left->right;
	
	r->left->right->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->left->type = NUMBER;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;
	
	r->left->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->type = POWER;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;
	
	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);
	
	r->left->right->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->right->type = NUMBER;
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
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = DIVIDE;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = -1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* sqrt(...) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = SQRT;
	r->left->right->parent = r->left;
	r->left->right->left = NULL;
	
	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->type = MINUS;
	r->left->right->right->parent = r->left->right;
	
	r->left->right->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->left->type = NUMBER;
	r->left->right->right->left->value = 1;
	r->left->right->right->left->sign = 1;
	r->left->right->right->left->parent = r->left->right->right;
	r->left->right->right->left->left = r->left->right->right->left->right = NULL;
	
	r->left->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->type = POWER;
	r->left->right->right->right->value = 0;
	r->left->right->right->right->sign = 1;
	r->left->right->right->right->parent = r->left->right->right;
	
	r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);
	
	r->left->right->right->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->right->right->type = NUMBER;
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
	r->type = MULTIPLY;
	r->sign = 1;
	r->parent = NULL;
	
	r->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->type = DIVIDE;
	r->left->sign = 1;
	r->left->parent = r;
	
	r->left->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->left->type = NUMBER;
	r->left->left->value = 1;
	r->left->left->sign = 1;
	r->left->left->parent = r->left;
	r->left->left->left = r->left->left->right = NULL;
	
	/* (v^2+1) */
	r->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->type = PLUS;
	r->left->right->parent = r->left;
	
	r->left->right->left = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->type = POWER;
	r->left->right->left->value = 0;
	r->left->right->left->sign = 1;
	r->left->right->left->parent = r->left->right;
	r->left->right->left->left = cloneTree(v, r->left->right->left);
	
	r->left->right->left->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->left->right->type = NUMBER;
	r->left->right->left->right->value = 2;
	r->left->right->left->right->sign = 1;
	r->left->right->left->right->parent = r->left->right->left;
	r->left->right->left->right->left = r->left->right->left->right->right = NULL;
	
	r->left->right->right = (NMAST *)malloc(sizeof(NMAST));
	r->left->right->right->type = NUMBER;
	r->left->right->right->value = 1;
	r->left->right->right->sign = 1;
	r->left->right->right->parent = r->left->right;
	r->left->right->right->left = r->left->right->right->right = NULL;
	
	r->right = dv;
	if(dv != NULL)
		dv->parent = r;
	return r;
}

/*
	Check if a tree contains variable x
	@param t the tree
	@param x variable to check if existed
*/
int isContainVar(NMAST *t, char x){
	
	if( (t==NULL) || (t->type==NUMBER) || (t->type==PI_TYPE) || (t->type==E_TYPE) )
		return 0;
		
	if( t->type == VARIABLE ){
		if(t->type == x)
			return 1;
		return 0;
	}
	
	return (isContainVar(t->left,x) || isContainVar(t->right, x) );
}
