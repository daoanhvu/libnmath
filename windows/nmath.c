#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>
#include <process.h>
#include "nmath.h"

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
}

void reset(Function *f, const char *str, int *error){
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

	f->variable[0] = 0;
	f->variable[1] = 0;
	f->variable[2] = 0;
	f->valLen = 0;

	for(i=0; i<f->prefixLen; i++)
		clearTree(&(f->prefix[i]));
	f->prefixLen = 0;
	(*error) = 0;
}

void setVariables(Function *f, char variable[], int l){
	int i;
	if(f == NULL)
		return;

	f->valLen = l;
	for(i=0;i<l;i++)
		f->variable[i] = variable[i];
	f->variable[i] = 0;
}

unsigned int __stdcall reduce_t(void *param){
	RParam *dp = (RParam *)param;
	NMAST *p;
	RParam this_param_left;
	RParam this_param_right;
	HANDLE thread_1 = 0, thread_2 = 0;
	this_param_left.error = this_param_right.error = 0;
	
	if((dp->t)==NULL){
		return 0;
	}

	/*
		If this node is has left child, and the left child is an operator or a function then we
		create a thread to reduce the left child.
	*/
	if( ((dp->t)->left) != NULL && ( contains(((dp->t)->left)->type, OPERATORS, OPERATOR_COUNT) || 
														contains(((dp->t)->left)->type, FUNCTIONS, FUNCTION_COUNT)) ){
		/*printf(" To Left \n", (t->left)->chr);*/
		this_param_left.t = (dp->t)->left;
		thread_1 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_left, 0, NULL);
	}
		
	/*
		If this node is has right child, and the right child is an operator or a function then we
		create a thread to reduce the right child.
	*/
	if( ((dp->t)->right) != NULL && ( contains(((dp->t)->right)->type, OPERATORS, OPERATOR_COUNT) || 
														contains(((dp->t)->right)->type, FUNCTIONS, FUNCTION_COUNT)) ){
		/*printf(" To Right\n", (t->right)->chr);*/
		this_param_right.t = (dp->t)->right;
		thread_2 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_right, 0, NULL);
	}
	if(thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}
	
	if(thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}

	if(this_param_left.error != 0){
		dp->error = this_param_left.error;
		return dp->error;
	}
	
	if(this_param_right.error != 0){
		dp->error = this_param_right.error;
		return dp->error;
	}
	
	/*
		We don't reduce a node if it's a variable, a number, PI, E
	*/
	if( ((dp->t)->type == VARIABLE) || ((dp->t)->type == NUMBER) || 
					((dp->t)->type == PI_TYPE) || ((dp->t)->type == E_TYPE))
		return dp->error;
		
	//if this node is an operator
	if( ((dp->t)->type == PLUS) || ((dp->t)->type == MINUS) || ((dp->t)->type == MULTIPLY)
					|| ((dp->t)->type == DIVIDE) || ((dp->t)->type == POWER) ){	
		if(((dp->t)->left!=NULL) && ((dp->t)->right!=NULL)){
			/* take care of special cases */
			if((dp->t)->type == PLUS){
				/* 0 + something */
				if( ((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 0.0 ){
				
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
						(dp->t)->priority = p->priority;
						(dp->t)->sign = p->sign;
						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						
						/* Now release p */
						free(p);
						p = NULL;
						return dp->error;
						
					}
				}
				
				/* something + 0 */
				if( ((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 0.0 ){
				
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
						return dp->error;
					}
				}
				
				//Left and right child of this node are null, 
				// I'm not sure that this piece of code can be reached
				if( ((dp->t)->left == NULL) || ((dp->t)->right == NULL) )
					return dp->error;
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
					return dp->error;
				}
				
				/* (1 * something) */
				if( (((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 1) ){
					
					clearTree(&((dp->t)->left));
					p = (dp->t)->right;

					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->chr = p->chr;
					(dp->t)->frValue = p->frValue;
					//(dp->t)->priority = p->priority;
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;

					free(p);
					
					return dp->error;
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
					return dp->error;
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
						return dp->error;
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
						
						return dp->error;
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
					 
					return dp->error;
				}
				
				// number ^ number, this is a trivial case
				if( ((dp->t)->left!=NULL) && ((dp->t)->right!=NULL) && (((dp->t)->left)->type == NUMBER) 
						&& (((dp->t)->right)->type == NUMBER) ){
					// printf("Process 2-opt function %d\n", (t->right)->function);
					(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
					
					if(dp->error != 0)
						return dp->error;
						
					(dp->t)->type = NUMBER;
					p = (dp->t)->left;
					free(p);
					p = (dp->t)->right;
					free(p);
					(dp->t)->left = (dp->t)->right = NULL;
					return dp->error;
				}
			} // END OPERATOR POWER
			
			// trivial cases
			if( (((dp->t)->left)->function == F_COE) && (((dp->t)->right)->function == F_COE) ){
				/*printf("doCalculate %c\n", t->type);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				if(dp->error != 0)
					return dp->error;
				(dp->t)->function = F_COE;
				p = (dp->t)->left;
				free(p);
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
				return dp->error;
			}
		}
	} // 2.0 get done here
	
	
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
						return dp->error;
					(dp->t)->type = NUMBER;
					if((dp->t)->left != NULL){
						p = (dp->t)->left;
						free(p);
					}
					p = (dp->t)->right;
					free(p);
					(dp->t)->left = (dp->t)->right = NULL;
					return dp->error;
				}
			}
			/*printf("End Process function %d\n", t->type);*/
		break;
			
		/* Power is an operator
		case POWER:
		break;
		*/
			
		case LOG:
			if( ((dp->t)->left!=NULL) && ((dp->t)->right!=NULL) && (((dp->t)->left)->type == NUMBER) 
					&& (((dp->t)->right)->type == NUMBER) ){
				/*printf("Process 2-opt function %d\n", (t->right)->function);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				
				if(dp->error != 0)
					return dp->error;
					
				(dp->t)->type = NUMBER;
				p = (dp->t)->left;
				free(p);
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
				return dp->error;
			}
		break;
	}
	return dp->error;
}

int reduce(Function *f, int *error){
	DParam dp;
	dp.t = *(f->prefix);
	dp.error = 0;
	reduce_t(&dp);
	return 0;
}

unsigned int __stdcall calc_t(void *param){
	RParam *dp = (RParam *)param;
	NMAST *t = dp->t;
	RParam this_param_left;
	RParam this_param_right;
	HANDLE thread_1 = 0, thread_2 = 0;
	int var_index = -1;

	this_param_left.error = this_param_right.error = 0;
	this_param_left.variables = this_param_right.variables = dp->variables;
	this_param_left.values = this_param_right.values = dp->values;
	if(t==NULL){
		return 0;
	}

	if(t->type == VARIABLE){
		var_index = isInArray(dp->variables, t->variable);
		dp->retv = dp->values[var_index];
		return dp->error;
	}
		
	if( (t->type == NUMBER) || (t->type == PI_TYPE) ||(t->type == E_TYPE) ){
		dp->retv = t->value;
		return dp->error;
	}

	this_param_left.t = t->left;
	thread_1 = (HANDLE)_beginthreadex(NULL, 0, &calc_t, (void*)&this_param_left, 0, NULL);
		
	this_param_right.t = t->right;
	thread_2 = (HANDLE)_beginthreadex(NULL, 0, &calc_t, (void*)&this_param_right, 0, NULL);
	
	if(thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}
	if(thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}

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
	return dp->error;
}

double calc(Function *f, double *values, int numOfValue, int *error){
	RParam rp;
	rp.error = 0;
	rp.t = *(f->prefix);
	rp.values = values;
	rp.variables = f->variable;
	calc_t((void*)&rp);
	return rp.retv;
}