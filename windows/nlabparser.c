#include <stdlib.h>
#include "nlablexer.h"
#include "common.h"

#define INCLEN 10

int parseFunct(TokenList *tokens, int *idxE){
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