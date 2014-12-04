#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

#include "StringUtil.h"
#include "nfunction.h"
#include "nlablexer.h"
#include "nlabparser.h"

using namespace nmath;

NFunction::NFunction() {
	
}

NFunction::~NFunction() {
	release();
}

/*
Parse the input string in object f to NMAST tree
*/
int NFunction::parse(const char *str, int len) {
	if (mLexer == 0) {
		mLexer = new NLabLexer(len);
	}
	else {
		mLexer->reset(len);
	}

	if (text != NULL) {
		delete[] text;
	}
	memcpy(text, str, len);
	textLen = len;

	errorCode = mLexer->lexicalAnalysis(text, textLen, 0);
	if (errorCode != NMATH_NO_ERROR) {
		return errorCode;
	}
	else
		errorColumn = mLexer->getErrorColumn();

	errorCode = mParser->parseFunctionExpression(*mLexer);
	if (errorCode == NMATH_NO_ERROR) {
		this->prefix = mParser->prefix();
		this->domain = mParser->domain();
		valLen = mParser->variableCount();
		if (valLen > 0)
			memcpy(variables, mParser->variables(), valLen);

		mParser->reset();
	} else
		errorColumn = mParser->getErrorColumn();

	return errorCode;
}

		
void NFunction::release() {
	if (text != NULL) {
		delete[] text;
		text = 0;
		textLen = 0;
	}
}

double NFunction::dcalc(double *values, int numOfValue) {
	DParam rp;
	rp.error = 0;
	rp.t = *(prefix->list);
	rp.values = values;
	memcpy(rp.variables, variables, 4);
	calc_t((void*)&rp);
	return rp.retv;
}

int NFunction::reduce() {
	DParam dp;
	dp.t = *(prefix->list);
	dp.error = 0;
	nmath::reduce_t(&dp);
	return 0;
}

#ifdef _WIN32
unsigned int __stdcall nmath::reduce_t(void *param){
	HANDLE thread_1 = 0, thread_2 = 0;
#else
void* nmath::reduce_t(void *param){
	pthread_t thrLeft, thrRight;
	int idThrLeft = -1, idThrRight = -1;
#endif
	DParam *dp = (DParam *)param;
	NMAST *p;
	NMAST *o1, *o2;
	DParam this_param_left;
	DParam this_param_right;
	this_param_left.error = this_param_right.error = 0;

	/* If the tree is NULL */
	if ((dp->t) == NULL){
		return 0;
	}

#ifdef _WIN32
	/*
	If this node is has left child, and the left child is an operator or a function then we
	create a thread to reduce the left child.
	*/
	if (((dp->t)->left) != NULL && isFunctionOROperator(((dp->t)->left)->type)){
		this_param_left.t = (dp->t)->left;
		thread_1 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_left, 0, NULL);
	}

	/*
	If this node is has right child, and the right child is an operator or a function then we
	create a thread to reduce the right child.
	*/
	if (((dp->t)->right) != NULL && isFunctionOROperator(((dp->t)->right)->type)){
		this_param_right.t = (dp->t)->right;
		thread_2 = (HANDLE)_beginthreadex(NULL, 0, &reduce_t, (void*)&this_param_right, 0, NULL);
	}
	if (thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}

	if (thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}
#else
	/*
	If this node is has left child, and the left child is an operator or a function then we
	create a thread to reduce the left child.
	*/
	if (((dp->t)->left) != NULL && isFunctionOROperator(((dp->t)->left)->type)){
		this_param_left.t = (dp->t)->left;
		idThrLeft = pthread_create(&thrLeft, NULL, reduce_t, (void*)(&this_param_left));
	}
	if (((dp->t)->right) != NULL && isFunctionOROperator(((dp->t)->right)->type)){
		this_param_right.t = (dp->t)->right;
		idThrRight = pthread_create(&thrRight, NULL, reduce_t, (void*)(&this_param_right));
	}
	if (idThrLeft == 0)
		pthread_join(thrLeft, NULL);
	if (idThrRight == 0)
		pthread_join(thrRight, NULL);
#endif

	if (this_param_left.error != 0){
		dp->error = this_param_left.error;
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	if (this_param_right.error != 0){
		dp->error = this_param_right.error;
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}
	/*************************************************************************************/

	/*
	We don't reduce a node if it's a variable, a NAME, a number, PI, E
	*/
	if (((dp->t)->type == VARIABLE) || ((dp->t)->type == NAME) || isConstant((dp->t)->type))
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif

	//if this node is an operator
	if (((dp->t)->type == PLUS) || ((dp->t)->type == MINUS) || ((dp->t)->type == MULTIPLY)
		|| ((dp->t)->type == DIVIDE) || ((dp->t)->type == POWER)) {
		if (((dp->t)->left != NULL) && ((dp->t)->right != NULL)) {

			/*****************************************************************/
			if (isConstant((dp->t)->right->type) &&
				((dp->t)->left->type == PLUS || (dp->t)->left->type == MINUS ||
				(dp->t)->left->type == MULTIPLY /*|| (dp->t)->left->type == DIVIDE */)) {

				o1 = dp->t;
				o2 = (dp->t)->left;
				if (o1->priority == o2->priority){
					if (isConstant(o2->left->type)) {
						switch (o1->type){
						case PLUS:
							o2->left->value += o1->right->value;
							break;

						case MINUS:
							o2->left->value -= o1->right->value;
							break;

						case MULTIPLY:
							o2->left->value *= o1->right->value;
							break;

							//case DIVIDE:
							//break;
						}

						if ((o1->parent != NULL) && (o1 == o1->parent->left)) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}
						o2->parent = o1->parent;
						dp->t = o2;

						//remove o1 from the current tree
						o1->parent = NULL;
						o1->left = NULL;
						clearTree(&o1);
						//free(o1);
					}
					else if (isConstant(o2->right->type)){
						switch (o1->type){
						case PLUS:
							o2->right->value += o1->right->value;
							break;

						case MINUS:
							o2->right->value -= o1->right->value;
							break;

						case MULTIPLY:
							o2->right->value *= o1->right->value;
							break;

							//case DIVIDE:
							//break;

						}

						if ((o1->parent != NULL) && o1 == o1->parent->left) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}

						//remove o1 from the current tree
						o2->parent = o1->parent;
						dp->t = o2;
						o1->parent = NULL;
						o1->left = NULL;
						clearTree(&o1);
						//free(o1);
					}

					if (o2->type == PLUS && o2->left->value < 0){
						o2->type = MINUS;
						o2->left->value = -(o2->left->value);
					}
				}

			}
			else if (isConstant((dp->t)->left->type) &&
				((dp->t)->right->type == PLUS || (dp->t)->right->type == MINUS ||
				(dp->t)->right->type == MULTIPLY /*|| (dp->t)->right->type == DIVIDE */)) {

				o1 = dp->t;
				o2 = (dp->t)->right;
				if (o1->priority == o2->priority){
					if (isConstant(o2->left->type)){
						switch (o1->type) {
						case PLUS:
							o2->left->value += o1->left->value;
							break;

						case MINUS:
							o2->left->value -= o1->left->value;
							break;

						case MULTIPLY:
							o2->left->value *= o1->left->value;
							break;

							//case DIVIDE:
							//break;
						}

						if ((o1->parent != NULL) && (o1 == o1->parent->left)) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}
						o2->parent = o1->parent;
						dp->t = o2;

						//remove o1 from the current tree
						o1->parent = NULL;
						o1->right = NULL;
						clearTree(&o1);
						//free(o1);
					}
					else if (isConstant(o2->right->type)){
						switch (o1->type){
						case PLUS:
							o2->right->value += o1->left->value;
							break;

						case MINUS:
							o2->right->value -= o1->left->value;
							break;

						case MULTIPLY:
							o2->right->value *= o1->left->value;
							break;

							//case DIVIDE:
							//break;

						}

						if ((o1->parent != NULL) && o1 == o1->parent->left) {
							//o1 is left child
							o1->parent->left = o2;
						}
						else if (o1->parent != NULL) {
							o1->parent->right = o2;
						}

						//remove o1 from the current tree
						o2->parent = o1->parent;
						dp->t = o2;
						o1->parent = NULL;
						o1->right = NULL;
						clearTree(&o1);
						//free(o1);
					}

					if (o2->type == PLUS && o2->left->value < 0){
						o2->type = MINUS;
						o2->left->value = -(o2->left->value);
					}
				}

			}

			/****************************************************************/

			/* take care of special cases */
			if ((dp->t)->type == PLUS){
				/* 0 + something */
				if ((((dp->t)->left)->type == NUMBER) && ((dp->t)->left)->value == 0.0) {

					//remove the left child (value 0)
					p = (dp->t)->left;
					free(p);
					(dp->t)->left = NULL;

					//move right child to its parent
					if ((dp->t)->right != NULL){
						p = (dp->t)->right;

						(dp->t)->type = p->type;
						(dp->t)->variable = p->variable;
						(dp->t)->value = p->value;
						(dp->t)->valueType = p->valueType;
						(dp->t)->frValue = p->frValue;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = p->sign;
						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;

						/* Now release p */
						free(p);
						p = NULL;
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}
				}

				/* something + 0 */
				if ((((dp->t)->right)->type == NUMBER) && (((dp->t)->right)->value == 0.0)){

					//remove the right child (value 0)
					p = (dp->t)->right;
					free(p);
					(dp->t)->right = NULL;

					if ((dp->t)->left != NULL){
						p = (dp->t)->left;

						(dp->t)->type = p->type;
						(dp->t)->variable = p->variable;
						(dp->t)->value = p->value;
						(dp->t)->valueType = p->valueType;
						(dp->t)->frValue = p->frValue;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = p->sign;

						/* NO copy parent */
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;

						/* Now release p */
						free(p);
						p = NULL;
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}
				}

				//Left and right child of this node are null, 
				// I'm not sure that this piece of code can be reached
				if (((dp->t)->left == NULL) || ((dp->t)->right == NULL))
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
			}

			if ((dp->t)->type == MULTIPLY){
				/* (0 * something) OR (something * 0) */
				if ((((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 0) ||
					(((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 0)){

					clearTree(&((dp->t)->left));
					clearTree(&((dp->t)->right));

					(dp->t)->type = NUMBER;
					(dp->t)->value = 0;
					(dp->t)->sign = 1;
					(dp->t)->variable = 0;

					/* MUST return here */
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}

				/* (1 * something) */
				if ((((dp->t)->left)->type == NUMBER && ((dp->t)->left)->value == 1)){

					clearTree(&((dp->t)->left));
					p = (dp->t)->right;

					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->variable = p->variable;
					(dp->t)->frValue = p->frValue;
					(dp->t)->priority = getPriorityOfType((dp->t)->type);
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;

					free(p);

#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}

				/* (something * 1) */
				if ((((dp->t)->right)->type == NUMBER && ((dp->t)->right)->value == 1)){
					clearTree(&((dp->t)->right));
					p = (dp->t)->left;
					(dp->t)->type = p->type;
					(dp->t)->value = p->value;
					(dp->t)->sign = p->sign;
					(dp->t)->variable = p->variable;
					(dp->t)->frValue = p->frValue;
					(dp->t)->priority = getPriorityOfType((dp->t)->type);
					(dp->t)->valueType = p->valueType;
					(dp->t)->left = p->left;
					(dp->t)->right = p->right;
					free(p);
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}
			}

			if ((dp->t)->type == POWER){
				if (((dp->t)->right)->type == NUMBER){
					if (((dp->t)->right)->value == 0.0){
						// something ^ 0 
						clearTree(&((dp->t)->left));
						clearTree(&((dp->t)->right));

						(dp->t)->type = NUMBER;
						(dp->t)->variable = 0;
						(dp->t)->valueType = TYPE_FLOATING_POINT;
						(dp->t)->value = 1.0;
						((dp->t)->frValue).numerator = 1;
						((dp->t)->frValue).denomerator = 1;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = 1;
						(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}

					if (((dp->t)->right)->value == 1.0){
						// something ^ 1 
						p = (dp->t)->right;
						free(p);

						p = (dp->t)->left;

						(dp->t)->type = p->type;
						(dp->t)->variable = p->variable;
						(dp->t)->valueType = p->valueType;
						(dp->t)->value = p->value;
						(dp->t)->frValue = p->frValue;
						(dp->t)->priority = getPriorityOfType((dp->t)->type);
						(dp->t)->sign = p->sign;
						(dp->t)->left = p->left;
						(dp->t)->right = p->right;
						free(p);

#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif
					}
				}

				// 0 ^ something
				if ((((dp->t)->left)->type == NUMBER) && (((dp->t)->left)->value == 0.0)){

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

#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}

				// number ^ number, this is a trivial case
				if (((dp->t)->left != NULL) && ((dp->t)->right != NULL) && isConstant(((dp->t)->left)->type)
					&& isConstant(((dp->t)->right)->type)){
					// printf("Process 2-opt function %d\n", (t->right)->type);
					(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));

					if (dp->error != 0)
#ifdef _WIN32
						return dp->error;
#else
						return &(dp->error);
#endif

					(dp->t)->type = NUMBER;
					p = (dp->t)->left;
					free(p);
					p = (dp->t)->right;
					free(p);
					(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				}
			} // END OPERATOR POWER

			// trivial cases
			if (isConstant(((dp->t)->left)->type) && isConstant(((dp->t)->right)->type)){
				/*printf("doCalculate %c\n", t->type);*/
				(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				if (dp->error != 0)
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				(dp->t)->type = NUMBER;
				p = (dp->t)->left;
				free(p);
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
				return dp->error;
#else
				return &(dp->error);
#endif
			}
		}
	}

	/*
	Right now, I dont take case specicial case for functions like SIN(PI), LN(E)
	*/
	switch ((dp->t)->type){
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
		if ((dp->t)->right != NULL){
			if (((dp->t)->right)->type == NUMBER){
				(dp->t)->value = doCalculate(0, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
				if (dp->error != 0)
#ifdef _WIN32
					return dp->error;
#else
					return &(dp->error);
#endif
				(dp->t)->type = NUMBER;
				if ((dp->t)->left != NULL){
					p = (dp->t)->left;
					free(p);
				}
				p = (dp->t)->right;
				free(p);
				(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
				return dp->error;
#else
				return &(dp->error);
#endif
			}
		}
		/*printf("End Process function %d\n", t->type);*/
		break;

		/* Power is an operator
		case POWER:
		break;
		*/

	case LOG:
		if (((dp->t)->left != NULL) && ((dp->t)->right != NULL) && isConstant(((dp->t)->left)->type)
			&& isConstant(((dp->t)->right)->type)){
			/*printf("Process 2-opt function %d\n", (t->right)->type);*/
			(dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));

			if (dp->error != 0)
#ifdef _WIN32
				return dp->error;
#else
				return &(dp->error);
#endif

			(dp->t)->type = NUMBER;
			p = (dp->t)->left;
			free(p);
			p = (dp->t)->right;
			free(p);
			(dp->t)->left = (dp->t)->right = NULL;
#ifdef _WIN32
			return dp->error;
#else
			return &(dp->error);
#endif
		}
		break;
	}
#ifdef _WIN32
	return dp->error;
#else
	return &(dp->error);
#endif
}

#ifdef _WIN32
unsigned int __stdcall nmath::calc_t(void *param){
	HANDLE thread_1 = 0, thread_2 = 0;
#else
void* nmath::calc_t(void *param){
	pthread_t thrLeft, thrRight;
	int idThrLeft = -1, idThrRight = -1;
#endif
	DParam *dp = (DParam *)param;
	NMAST *t = dp->t;
	DParam this_param_left;
	DParam this_param_right;
	int var_index = -1;

	this_param_left.error = this_param_right.error = 0;
	this_param_left.variables[0] = this_param_right.variables[0] = dp->variables[0];
	this_param_left.variables[1] = this_param_right.variables[1] = dp->variables[1];
	this_param_left.variables[2] = this_param_right.variables[2] = dp->variables[2];
	this_param_left.variables[3] = this_param_right.variables[3] = dp->variables[3];
	//memcpy(this_param_left.variables, dp->variables, 4);
	//memcpy(this_param_right.variables, dp->variables, 4);
	this_param_left.values = this_param_right.values = dp->values;

	/* If the input tree is NULL, we do nothing */
	if (t == NULL) return 0;


	if (t->type == VARIABLE){
		var_index = isInArray(dp->variables, t->variable);
		dp->retv = (t->sign>0) ? (dp->values[var_index]) : (-dp->values[var_index]);
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	if ((t->type == NUMBER) || (t->type == PI_TYPE) || (t->type == E_TYPE)){
		dp->retv = t->value;
#ifdef _WIN32
		return dp->error;
#else
		return &(dp->error);
#endif
	}

	this_param_left.t = t->left;
	this_param_right.t = t->right;
#ifdef _WIN32
	thread_1 = (HANDLE)_beginthreadex(NULL, 0, &calc_t, (void*)&this_param_left, 0, NULL);
	thread_2 = (HANDLE)_beginthreadex(NULL, 0, &calc_t, (void*)&this_param_right, 0, NULL);
	if (thread_1 != 0){
		WaitForSingleObject(thread_1, INFINITE);
		CloseHandle(thread_1);
	}
	if (thread_2 != 0){
		WaitForSingleObject(thread_2, INFINITE);
		CloseHandle(thread_2);
	}
#else
	idThrLeft = pthread_create(&thrLeft, NULL, calc_t, (void*)&this_param_left);
	idThrRight = pthread_create(&thrRight, NULL, calc_t, (void*)&this_param_right);
	if (idThrLeft == NMATH_NO_ERROR){
		pthread_join(thrLeft, NULL);
	}
	if (idThrRight == NMATH_NO_ERROR){
		pthread_join(thrRight, NULL);
	}
#endif
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

	dp->retv = t->sign * doCalculate(this_param_left.retv, this_param_right.retv, t->type, &(dp->error));
#ifdef _WIN32
	return dp->error;
#else
	return &(dp->error);
#endif
}