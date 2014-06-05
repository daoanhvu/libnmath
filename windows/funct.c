#include "funct.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>
#include <process.h>

#define UPSIZE 4

#define PUSH 0
#define POP 1
#define ADD 3

#define ADDRSIN 0
#define ADDRCOS 1
#define ADDRTAN 2
#define ADDRCOTAN 3
#define ADDRLN 4
#define ADDRASIN 5
#define ADDRACOS 6
#define ADDRATAN 7
#define ADDRSQRT 8
/*
#define FMAXLEN 6

typedef struct tagFTable{
	int f;
	char fname[FMAXLEN];
}FTABLE;

FTABLE gFtable[] = {	{SIN,{'s','i','n','\0'}},
							{COS,{'c','o','s','\0'}},
							{TAN,{'t','a','n','\0'}},
							{COTAN,{'c','o','t','a','n','\0'}},
							{LN,{'l','n','\0'}},
							{ASIN,{'a','s','i','n','\0'}},
							{ACOS,{'a','c','o','s','\0'}},
							{ATAN,{'a','t','a','n','\0'}},
							{SQRT,{'s','q','r','t','\0'}},
							{POW,{'p','o','w','\0'}},
							{LOG,{'l','o','g','\0'}}
							};
							
void getFName(int f, char *fname){
	int i;
	for(i=0; i<11; i++){
		if(gFtable[i].f == f){
			memcpy(fname, gFtable[i].fname, FMAXLEN);
			return;
		}
	}
}
*/

/*
	Return -1 if c is not a variable
	otherwise return the index of c
*/
int isVariable(Function *f, char c){
	if(c == f->variable[0])
		return 0;

	if(c == f->variable[1])
		return 1;

	if(c == f->variable[2])
		return 2;

	return -1;
}

int isInArray(char *vars, char c){
	if(c == vars[0])
		return 0;

	if(c == vars[1])
		return 1;

	if(c == vars[2])
		return 2;

	return -1;
}

char getPriority(char c){
	switch(c){
		case '+':
		case '-':
			return PLUS_MINUS_PRIORITY;
		case '*':
		case '/':
			return MUL_DIV_PRIORITY;
		case '^':
			return FUNCTION_PRIORITY;
	}

	return 0;
}

void addItem2Prefix(Function *f, TNode *item){
	if(f==NULL)
		return;

	if( f->prefixLen >= f->prefixAllocLen ){
		f->prefixAllocLen += UPSIZE;
		f->prefix = (TNode**)realloc(f->prefix, sizeof(TNode*) * (f->prefixAllocLen) );
	}

	f->prefix[f->prefixLen] = item;
	f->prefixLen++;
}

void addValue2Prefix(Function *f, int chr, char function, char priority, double value){
	TNode *item;

	if(f==NULL)
		return;

	item = (TNode*)malloc(sizeof(TNode));
	item->valueType = TYPE_FLOATING_POINT;
	item->sign = 1;
	item->function = function;
	item->chr = chr;
	item->value = value;
	item->left = item->right = NULL;

	if( f->prefixLen >= f->prefixAllocLen ){
		f->prefixAllocLen += UPSIZE;
		f->prefix = (TNode**)realloc(f->prefix, sizeof(TNode*) * (f->prefixAllocLen) );
	}

	f->prefix[f->prefixLen] = item;
	f->prefixLen++;
}

/* 
 * The 1st parameter MUST be a pointer, NOT a reference to static variable
 *
 * */
void pushItem2Stack(StackItem ***st, int *top, int *allocLen, StackItem *item){
	if( (*top) >= ( (*allocLen)-1)){
		(*allocLen) += UPSIZE;
		(*st) = (StackItem**)realloc(*st, sizeof(StackItem*) * (*allocLen) );
	}
	(*top)++;
	(*st)[(*top)] = item;
}

void push2Stack(StackItem ***st, int *top, int *allocLen, int chr,
		char function, char priority){
	StackItem *item;

	item = (StackItem*)malloc(sizeof(StackItem));
	item->function = function;
	item->chr = chr;
	item->priority = priority;

	if( (*top) >= ((*allocLen)-1)){
		(*allocLen) += UPSIZE;
		(*st) = (StackItem**)realloc(*st, sizeof(StackItem*) * (*allocLen) );
	}
	(*top)++;
	(*st)[(*top)] = item;
}

StackItem* popFromStack(StackItem **st, int *top){
	StackItem *p;

	if(st == NULL || ( (*top) < 0))
		return NULL;

	p = st[(*top)];
	st[(*top)] = NULL;
	(*top)--;
	return p;

}

/*=================================================================*/
void clearTree(TNode **prf){
	
	if((*prf) == NULL)
		return;
	
	if((*prf)->left != NULL)
		clearTree(&(*prf)->left);
	if((*prf)->right != NULL)
		clearTree(&(*prf)->right);
		
	free(*prf);
	(*prf) = NULL;
}

int clearStack(StackItem **ls, int len){
	int i;
	StackItem *p;

	if(ls==NULL)
		return 0;

	for(i=0; i<len; i++){
		p = ls[i];
		ls[i] = NULL;
		free(p);
	}
	return i;
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

void addFunction2Tree(Function *f, StackItem * stItm){
	TNode *itm = NULL;
	
	switch(stItm->function){
		case F_OPT:
			itm = (TNode*)malloc(sizeof(TNode));
			itm->valueType = TYPE_FLOATING_POINT;
			itm->sign = 1;
			itm->chr = stItm->chr;
			itm->function = stItm->function;
			itm->priority = stItm->priority;
			itm->parent = NULL;
			itm->left = f->prefix[f->prefixLen-2];
			itm->right = f->prefix[f->prefixLen-1];
			if((f->prefix[f->prefixLen-2])!=NULL)
				(f->prefix[f->prefixLen-2])->parent = itm;
			if((f->prefix[f->prefixLen-1])!=NULL)
				(f->prefix[f->prefixLen-1])->parent = itm;
				
			f->prefix[f->prefixLen-2] = itm;
			f->prefix[f->prefixLen-1] = NULL;
			f->prefixLen--;
		break;
		case F_FUNCT:
			switch(stItm->chr){
				case ASIN:
				case ACOS:
				case ATAN:
				case SIN:
				case COS:
				case TAN:
				case COTAN:
				case SQRT:
				case LN:
					itm = (TNode*)malloc(sizeof(TNode));
					itm->valueType = TYPE_FLOATING_POINT;
					itm->sign = 1;
					itm->chr = stItm->chr;
					itm->function = stItm->function;
					itm->priority = stItm->priority;
					itm->parent = NULL;
					itm->left = NULL;
					itm->right = f->prefix[f->prefixLen-1];
					if((f->prefix[f->prefixLen-1])!=NULL)
						(f->prefix[f->prefixLen-1])->parent = itm;
						
					f->prefix[f->prefixLen-1] = itm;
				break;
										
				//case POW:
				case LOG:
					itm = (TNode*)malloc(sizeof(TNode));
					itm->valueType = TYPE_FLOATING_POINT;
					itm->sign = 1;
					itm->chr = stItm->chr;
					itm->function = stItm->function;
					itm->priority = stItm->priority;
					itm->parent = NULL;
					itm->left = f->prefix[f->prefixLen-2];
					itm->right = f->prefix[f->prefixLen-1];
					if((f->prefix[f->prefixLen-2])!=NULL)
						(f->prefix[f->prefixLen-2])->parent = itm;
					if((f->prefix[f->prefixLen-1])!=NULL)
						(f->prefix[f->prefixLen-1])->parent = itm;
						
					f->prefix[f->prefixLen-2] = itm;
					f->prefix[f->prefixLen-1] = NULL;
					f->prefixLen--;
				break;
			}
		break;
	}
}

/*use PRN to convert function to prefix format*/
int parseFunct(Function *f, int *idxE){
	int i=0, j =0, error, top=-1, allocLen=0;
	double val;
	TNode *itm = NULL;
	StackItem **stack = NULL;
	StackItem *stItm = NULL;

	/*test*/
	/*int step = 1;*/

	if(f == NULL)
		return -1;

	/* Clear prefix tree if any*/
	if(f->prefixLen > 0){
		for(i=0; i<f->prefixLen; i++)
			clearTree(&(f->prefix[i]));
		f->prefixLen = 0;
	}

	while(i < f->len){
		*idxE = i;
		switch(f->str[i]){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':
				j = i + 1;
				while( ( (f->str[j]>=48) && (f->str[j]<=57) ) || (f->str[j]==46) )
					j++;

				val = parseDouble(f->str, i, j, &error);

				if(val == 0 && error < 0){
					/*printf("ERROR 0");*/
					clearStack(stack, top+1);
					free(stack);
					*idxE = j;
					return error;
				}
				itm = (TNode*)malloc(sizeof(TNode));
				itm->valueType = TYPE_FLOATING_POINT;
				itm->sign = 1;
				itm->left = itm->right = itm->parent = NULL;
				itm->value = val;
				itm->chr = COE;
				itm->function = F_COE; /*this is a coefficient*/
				addItem2Prefix(f, itm);
				/*printf("%d Add %lf to Prefix \n\n", step++, itm->value);*/
				i = j;
				break;

			case 'e':
				itm = (TNode*)malloc(sizeof(TNode));
				itm->left = itm->right = itm->parent = NULL;
				itm->value = E;
				itm->chr = 'e';
				itm->function = F_CONSTAN; /*this is a coefficient*/
				addItem2Prefix(f, itm);
				i++;
				break;

			case 'p':
				if(f->len <= i+2){
					clearStack(stack, top+1);
					free(stack);
					return ERROR_SYNTAX;
				}

				if(f->str[i+1]=='i'){
					/*printf("6 Stack: %d \n", f->stack);*/
					itm = (TNode*)malloc(sizeof(TNode));
					itm->left = itm->right = itm->parent = NULL;
					itm->value = PI;
					itm->chr = PI_FLG;
					itm->function = F_CONSTAN; /*this is a coefficient*/
					addItem2Prefix(f, itm);
					/*printf("6 Add PI to Prefix \n");*/
					i += 2;
					break;
				}
				clearStack(stack, top+1);
				free(stack);
				return ERROR_SYNTAX;
				break;

			case '+':
			case '-':
			case '*':
			case '/':
			case '^':
				if(top >= 0){
					stItm = stack[top];
					/*while((stItm->function == F_OPT || stItm->function == F_FUNCT ) && (stItm->priority) >= getPriority(f->str[i])){*/
					while((stItm->function == F_OPT ) && (stItm->priority) >= getPriority(f->str[i])){
						/*printf("1 Stack %d\n", f->stack);*/
						stItm = popFromStack(stack, &top);
						/*printf("Popped %c from Stack (top: %d )\n", itm->chr, top);*/
						itm = (TNode*)malloc(sizeof(TNode));
						itm->left = itm->right = NULL;
						itm->chr = stItm->chr;
						itm->function = stItm->function;
						itm->priority = stItm->priority;
						itm->left = f->prefix[f->prefixLen-2];
						itm->right = f->prefix[f->prefixLen-1];
						
						if((f->prefix[f->prefixLen-2])!=NULL)
							(f->prefix[f->prefixLen-2])->parent = itm;
						if((f->prefix[f->prefixLen-1])!=NULL)
							(f->prefix[f->prefixLen-1])->parent = itm;
						
						f->prefix[f->prefixLen-2] = itm;
						f->prefix[f->prefixLen-1] = NULL;
						f->prefixLen--;
						free(stItm);
						if(top < 0)
							break;

						stItm = stack[top];
					}
				}

				//push operation o1 into stack
				stItm = (StackItem*)malloc(sizeof(StackItem));
				stItm->chr = f->str[i];
				stItm->priority = getPriority(f->str[i]);
				stItm->function = F_OPT;
				
				/*
				if( (stItm->chr)=='^' ) // <-- pow is a function, not operator
					stItm->function = F_FUNCT;
				*/
				
				pushItem2Stack(&stack, &top, &allocLen, stItm);
				/*printf("Pushed %c (priority: %d) to Stack (top:%d)\n", f->str[i], stItm->priority, top);*/
				i++;
				break;

			case '(':/*If it an open parentheses then put it to stack*/
				/*pushValue(StackItem**, char flag, char chr, char isFunction, char priority, double value)*/
				push2Stack(&stack, &top, &allocLen, OPN, F_PARENT, PRIORITY_0);
				/*printf("%d Pushed '(' to Stack \n\n", step++);*/
				i++;
				break;

			case ')':
				stItm = popFromStack(stack, &top);
				/*printLog(step++, itm, POP);*/

				if(stItm == NULL){
					clearStack(stack, top+1);
					free(stack);
					return ERROR_SYNTAX;
				}

				/*  */
				while(stItm!=NULL && stItm->function != F_PARENT && stItm->function != F_FUNCT){
					addFunction2Tree(f, stItm);
					free(stItm);
					stItm = popFromStack(stack, &top);
					/*printf("%d Popped %d from Stack \n\n", step++, itm->flag);*/
				}

				/*check if Open parenthese missing*/
				if(stItm==NULL){
					/*printf("%d Stack top = %d to prefix \n\n", step++, top);*/
					free(stack);
					return ERROR_PARENTHESE_MISSING;
				}

				if(stItm->function == F_FUNCT){
					addFunction2Tree(f, stItm);
				}
				free(stItm);
				i++;
				break;

			case 'a':
				if(f->len <= i+5){
					clearStack(stack, top+1);
					free(stack);
					return ERROR_SYNTAX;
				}

				if(f->str[i+1]=='s' && f->str[i+2]=='i' && f->str[i+3]=='n'
						&& f->str[i+4]=='(' ){
					push2Stack(&stack, &top, &allocLen, ASIN, F_FUNCT, FUNCTION_PRIORITY);
					/*printf("6 Pushed asin to Stack %d\n\n", f->stack);*/
					i += 5;
					break;
				}
				
				if(f->str[i+1]=='c' && f->str[i+2]=='o' && f->str[i+3]=='s'
						&& f->str[i+4]=='(' ){
					push2Stack(&stack, &top, &allocLen, ACOS, F_FUNCT, FUNCTION_PRIORITY);
					/*printf("6 Pushed asin to Stack %d\n\n", f->stack);*/
					i += 5;
					break;
				}
				
				if(f->str[i+1]=='t' && f->str[i+2]=='a' && f->str[i+3]=='n'
						&& f->str[i+4]=='(' ){
					/*printf("6 Stack: %d \n", f->stack);*/
					push2Stack(&stack, &top, &allocLen, ATAN, F_FUNCT, FUNCTION_PRIORITY);
					/*printf("6 Pushed asin to Stack %d\n\n", f->stack);*/
					i += 5;
					break;
				}
				
				clearStack(stack, top+1);
				free(stack);
				return -2;

			case 's':
				if(f->len <= i+4){
					clearStack(stack, top+1);
					free(stack);
					return ERROR_SIN_SQRT;
				}

				if(f->str[i+1]=='i' && f->str[i+2]=='n' && f->str[i+3]=='('){
					push2Stack(&stack, &top, &allocLen, SIN, F_FUNCT, FUNCTION_PRIORITY);
					/*printf("Pushed sin to Stack\n");*/
					i += 4;
					break;
				}
				
				if(f->str[i+1]=='e' && f->str[i+2]=='c' && f->str[i+3]=='('){
					push2Stack(&stack, &top, &allocLen, SEC, F_FUNCT, FUNCTION_PRIORITY);
					/*printf("(i:%d)Pushed sin to Stack\n", i);*/
					i += 4;
					break;
				}

				/*sqrt*/
				if(f->len <= i+5){
					/*printf("ERROR 2");*/
					clearStack(stack, top+1);
					free(stack);
					return ERROR_SIN_SQRT;
				}

				if(f->str[i+1]=='q' && f->str[i+2]=='r' && f->str[i+3]=='t' &&
						f->str[i+4]=='('){
					push2Stack(&stack, &top, &allocLen, SQRT, F_FUNCT, FUNCTION_PRIORITY);
					i += 5;
					break;
				}
				/* Syntax error */
				/*printf("ERROR 3");*/
				clearStack(stack, top+1);
				free(stack);
				return ERROR_SIN_SQRT;

			case 'c':
				if(f->len <= i+4){
					clearStack(stack, top+1);
					free(stack);
					return -2;
				}

				if(f->str[i+1]=='o' && f->str[i+2]=='s' && f->str[i+3]=='('){
					push2Stack(&stack, &top, &allocLen, COS, F_FUNCT, FUNCTION_PRIORITY);
					i += 4;
					break;
				}

				/* cotan*/
				if(f->len <= i+6){
					clearStack(stack, top+1);
					free(stack);
					return -2;
				}
				if(f->str[i+1]=='o' && f->str[i+2]=='t' && f->str[i+3]=='a' &&
					f->str[i+4]=='n' && f->str[i+4]=='('){
					push2Stack(&stack, &top, &allocLen, COTAN, F_FUNCT, FUNCTION_PRIORITY);
					i += 6;
					break;
				}
				clearStack(stack, top+1);
				free(stack);
				return -2;

			case 't':
				if(f->len <= i+4){
					clearStack(stack, top+1);
					free(stack);
					return -2;
				}

				if(f->str[i+1]=='a' && f->str[i+2]=='n' && f->str[i+3]=='('){
					push2Stack(&stack, &top, &allocLen, TAN, F_FUNCT, FUNCTION_PRIORITY);
					i += 4;
					break;
				}
				clearStack(stack, top+1);
				free(stack);
				return -2;

			case 'l':
				if(f->len <= i+3){
					clearStack(stack, top+1);
					free(stack);
					return -2;
				}

				if(f->str[i+1]=='n' && f->str[i+2]=='('){
					push2Stack(&stack, &top, &allocLen, LN, F_FUNCT, FUNCTION_PRIORITY);
					i += 3;
					break;
				}

				/* log(
				 *
				 * */
				if(f->len <= i+4){
					clearStack(stack, top+1);
					free(stack);
					return -2;
				}
				if(f->str[i+1]=='o' && f->str[i+2]=='g' && f->str[i+3]=='('){
					push2Stack(&stack, &top, &allocLen, LOG,F_FUNCT, FUNCTION_PRIORITY);
					/*printf("%d Pushed log to stack(stack top=%d) \n\n", step++, top);*/
					i += 4;
					break;
				}
				clearStack(stack, top+1);
				free(stack);
				return ERROR_LOG;

			case ',':
				i++;
				break;

			default: //variables
				if(isVariable(f, f->str[i]) >=0 ){
					addValue2Prefix(f, f->str[i], F_VAR, PRIORITY_0, 0);
					i++;
					/*printf("Variable %c next char %c \n", f->str[i], f->str[i+1]);
					clearStack(stack, top+1);
					free(stack);
					return -2;*/
					break;
				}
				/*printf("ERROR 4: i=%d; char=%c", i, f->str[i]);*/
				clearStack(stack, top+1);
				free(stack);
				return -2;
		}//end switch

		/*
		printf("Stack: ");
		printStack(stack, top+1);
		printf("Result: ");
		printStack(f->prefix, f->prefixLen);
		*/
	}//end while

	while(top >= 0){
		stItm = popFromStack(stack, &top);
		/*printLog(step++, itm, POP);*/
		if(stItm->chr == OPN || (stItm->function==F_FUNCT)){
			free(stItm);
			clearStack(stack, top+1);
			free(stack);
			return ERROR_PARENTHESE_MISSING; 
		}
		
		addFunction2Tree(f, stItm);
		free(stItm);
	}
	free(stack);
	return 0;
}

double logab(double a, double b, int *error){
	(*error) = 0;
	if( (b > 0.0) && (a > 0.0) && (a != 1.0))
		return log(b)/log(a);

	(*error) = ERROR_LOG;
	return 0;
}

double doCalculate(double val1, double val2, char opt, int *error){
	(*error) = 0;
	switch(opt){
		case PLUS:
			/*printf("%lf+%lf=%lf\n", val1, val2, val1 + val2);*/
			return val1 + val2;

		case MINUS:
			/*printf("%lf-%lf=%lf\n", val1, val2, val1 - val2);*/
			return val1 - val2;

		case MUL:
			/*printf("%lf*%lf=%lf\n", val1, val2, val1 * val2);*/
			return val1 * val2;

		case DIV:
			if(val2 == 0.0){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return val1/val2;

		case POW:
			return pow(val1, val2);

		case LOG:
			return logab(val1, val2, error);

		case LN:
			return log(val2);

		case SIN:
			return sin(val2);

		case ASIN:
			return asin(val2);

		case COS:
			return cos(val2);

		case ACOS:
			return acos(val2);

		case COTAN:
			if(val2==0 || val2 == PI){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return cos(val2)/sin(val2);

		case TAN:
			if(val2==PI/2){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return tan(val2);

		case ATAN:
			return atan(val2);
		
		case SEC:
			if(val2==PI/2){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return 1/cos(val2);

		case SQRT:
			if(val2 < 0){
				(*error) = ERROR_OUT_OF_DOMAIN;
				return 0;
			}
			return sqrt(val2);
	}
	return 0;
}

unsigned int __stdcall reduce_t(void *param){
	RParam *dp = (RParam *)param;
	TNode *p;
	RParam this_param_left;
	RParam this_param_right;
	HANDLE thread_1 = 0, thread_2 = 0;
	this_param_left.error = this_param_right.error = 0;
	
	if((dp->t)==NULL){
		return 0;
	}

	if( ((dp->t)->left) != NULL && ( (((dp->t)->left)->function==F_OPT) ||
		(((dp->t)->left)->function==F_FUNCT)) ){
		/*printf(" To Left \n", (t->left)->chr);*/
		this_param_left.t = (dp->t)->left;
		thread_1 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_left, 0, NULL);
	}
		
	if( ((dp->t)->right) != NULL && ( (((dp->t)->right)->function==F_OPT) || 
		(((dp->t)->right)->function==F_FUNCT)) ){
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
		
	if( ((dp->t)->function == F_VAR) || ((dp->t)->function == F_COE) || ((dp->t)->function == F_CONSTAN) )
		return dp->error;
		
	if( (dp->t)->function == F_OPT ){	
		if(((dp->t)->left!=NULL) && ((dp->t)->right!=NULL)){
			/* take care of special cases */
			if((dp->t)->chr == PLUS){
				/* 0 + something */
				if( ((dp->t)->left)->function == F_COE && ((dp->t)->left)->value == 0 ){
					p = (dp->t)->left;
					free(p);
					(dp->t)->left = NULL;
					
					if((dp->t)->right != NULL){
						p = (dp->t)->right;
						
						(dp->t)->function = p->function;
						(dp->t)->chr = p->chr;
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
				if( ((dp->t)->right)->function == F_COE && ((dp->t)->right)->value == 0 ){
					p = (dp->t)->right;
					free(p);
					(dp->t)->right = NULL;
					
					if((dp->t)->left != NULL){
						p = (dp->t)->left;
						
						(dp->t)->function = p->function;
						(dp->t)->chr = p->chr;
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
				
				if( ((dp->t)->left == NULL) || ((dp->t)->right == NULL) )
					return dp->error;
			}
			
			if((dp->t)->chr == MUL){
				/* 0 * something */
				if( (((dp->t)->left)->function == F_COE && ((dp->t)->left)->value == 0) ||
						(((dp->t)->right)->function == F_COE && ((dp->t)->right)->value == 0) ){
					
					clearTree(&((dp->t)->left));
					clearTree(&((dp->t)->right));
					
					(dp->t)->function = F_COE;
					(dp->t)->value = 0;
					(dp->t)->sign = 1;
					
					/* MUST return here */
					return dp->error;
				}
				
				/* (1 * something) */
				if( (((dp->t)->left)->function == F_COE && ((dp->t)->left)->value == 1) ){
					clearTree(&((dp->t)->left));
					p = (dp->t)->right;

					(dp->t)->function = p->function;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->chr = p->chr;
					(dp->t)->frValue = p->frValue;
					(dp->t)->priority = p->priority;
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;

					free(p);
					
					return dp->error;
				}

				/* (something * 1) */
				if( (((dp->t)->right)->function == F_COE && ((dp->t)->right)->value == 1) ){
					clearTree(&((dp->t)->right));
					p = (dp->t)->left;
					(dp->t)->function = p->function;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->chr = p->chr;
					(dp->t)->frValue = p->frValue;
					(dp->t)->priority = p->priority;
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;
					free(p);
					return dp->error;
				}
			}
			
			if((dp->t)->chr == POW){
				if(((dp->t)->right)->function == F_COE){
					if( ((dp->t)->right)->value == 0.0 ){
						// something ^ 0 
						clearTree(&((dp->t)->left));
						clearTree(&((dp->t)->right));
						
						(dp->t)->function = F_COE;
						(dp->t)->chr = 0;
						(dp->t)->valueType = TYPE_FLOATING_POINT;
						(dp->t)->value = 1;
						((dp->t)->frValue).numerator = 0;
						((dp->t)->frValue).denomerator = 1;
						(dp->t)->priority = COE_VAL_PRIORITY;
						(dp->t)->sign = 1;
						(dp->t)->left = (dp->t)->right = NULL;
						return dp->error;
					}
					
					if( ((dp->t)->right)->value == 1.0 ){
						// something ^ 1 
						p = (dp->t)->right;
						free(p);
						
						p = (dp->t)->left;
						
						(dp->t)->function = p->function;
						(dp->t)->chr = p->chr;
						(dp->t)->valueType = p->valueType;
						(dp->t)->value = p->value;
						(dp->t)->frValue = p->frValue;
						(dp->t)->priority = p->priority;
						(dp->t)->sign = p->sign;
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						
						free(p);
						
						return dp->error;
					}
				}
				
				if( ((dp->t)->left!=NULL) && ((dp->t)->right!=NULL) && (((dp->t)->left)->function == F_COE) 
						&& (((dp->t)->right)->function == F_COE) ){
					// printf("Process 2-opt function %d\n", (t->right)->function);
					(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->chr, &(dp->error));
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
			
			if( (((dp->t)->left)->function == F_COE) && (((dp->t)->right)->function == F_COE) ){
				/*printf("doCalculate %c\n", t->chr);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->chr, &(dp->error));
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
	}
	
	if( ((dp->t)->function == F_FUNCT)){
		switch( (dp->t)->chr ){
			case SIN:
			case COS:
			case TAN:
			case ASIN:
			case ACOS:
			case ATAN:
			case LN:
			case SQRT:
				/*printf("Process function %d\n", t->chr);*/
				if( (dp->t)->right != NULL){
					if(((dp->t)->right)->function==F_COE){
						(dp->t)->value = doCalculate(0, ((dp->t)->right)->value, (dp->t)->chr, &(dp->error));
						if(dp->error != 0)
							return dp->error;
						(dp->t)->function = F_COE;
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
				/*printf("End Process function %d\n", t->chr);*/
			break;
			/*
			case POW:
				if((t->right)->function == F_COE){
					if( (t->right)->value == 0.0 ){
						// something ^ 0 
						clearTree(&(t->left));
						clearTree(&(t->right));
						
						t->function = F_COE;
						t->chr = 0;
						t->valueType = TYPE_FLOATING_POINT;
						t->value = 0;
						(t->frValue).numerator = 0;
						(t->frValue).denomerator = 1;
						t->priority = COE_VAL_PRIORITY;
						t->sign = 1;
						t->left = t->right = NULL;
						return dp->error;
					}
					
					if( (t->right)->value == 1.0 ){
						// something ^ 1 
						p = t->right;
						free(p);
						
						p = t->left;
						
						t->function = p->function;
						t->chr = p->chr;
						t->valueType = p->valueType;
						t->value = p->value;
						t->frValue = p->frValue;
						t->priority = p->priority;
						t->sign = p->sign;
						t->left = p->left;
						t->right = p->right;
						
						free(p);
						
						return dp->error;
					}
				}
				
				if( (t->left!=NULL) && (t->right!=NULL) && ((t->left)->function == F_COE) 
						&& ((t->right)->function == F_COE) ){
					// printf("Process 2-opt function %d\n", (t->right)->function);
					t->value = doCalculate((t->left)->value, (t->right)->value, t->chr, &(dp->error));
					if(dp->error != 0)
						return dp->error;
					t->function = F_COE;
					p = t->left;
					free(p);
					p = t->right;
					free(p);
					t->left = t->right = NULL;
					return dp->error;
				}
			break;
			*/
			
			case LOG:
				if( ((dp->t)->left!=NULL) && ((dp->t)->right!=NULL) && (((dp->t)->left)->function == F_COE) 
						&& (((dp->t)->right)->function == F_COE) ){
					/*printf("Process 2-opt function %d\n", (t->right)->function);*/
					(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->chr, &(dp->error));
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
			break;
		}
		
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

/*
	This function should be called after reducing
*/
unsigned int __stdcall calc_t(void *param){
	RParam *dp = (RParam *)param;
	TNode *t = dp->t;
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

	if(t->function == F_VAR){
		var_index = isInArray(dp->variables, t->chr);
		dp->retv = dp->values[var_index];
		return dp->error;
	}
		
	if( (t->function == F_COE) || (t->function == F_CONSTAN) ){
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
		
	dp->retv = doCalculate(this_param_left.retv, this_param_right.retv, t->chr, &(dp->error));
	/*printf("Return value %lf \n", dp->retv);*/
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

void nodeToString(TNode *t, char *str, int len, int *newlen){
	char tmp[32];
	int i, l;
	double dval;
	
	if(t->left != NULL)
		nodeToString(t->left, str, len, newlen);
				
	if(t->right != NULL)
		nodeToString(t->right, str, len, newlen);
	
	/* These lines of code will slow down the process */	
	dval = t->value - (long)(t->value);
	/* ======================*/
	
	if( (*newlen)<=0 ){
		if(dval == 0.0)
			l = sprintf(tmp, "%d;%d;%ld", t->function, (long)(t->value));
		else
			l = sprintf(tmp, "%d;%d;%lf", t->function, t->chr, t->value);
	}
	else{
		if(dval==0.0)
			l = sprintf(tmp, "|%d;%d;%ld", t->function, (long)(t->value));
		else
			l = sprintf(tmp, "|%d;%d;%lf", t->function, t->chr, t->value);
	}
	
	for(i=0; i<=l;i++)
		str[(*newlen)+i] = tmp[i];
	/*printf("%s len: %d\n", tmp, l);*/
	(*newlen) += l;
}


void releaseFunct(Function *f){
	int i;
	TNode **temp;
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
}
