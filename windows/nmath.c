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