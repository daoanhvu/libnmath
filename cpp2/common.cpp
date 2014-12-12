#include <stdlib.h>
#include <math.h>
#include "common.h"

#define POOL_CAPACITY 32

const int FUNCTIONS[] = {SIN, COS, TAN, COTAN, ASIN, ACOS, ATAN, LOG, LN, SQRT};
const int FUNCTION_COUNT = 10;

const int OPERATORS[] = {PLUS,MINUS,MULTIPLY,DIVIDE,POWER};
const int OPERATOR_COUNT = 5;

const int COMPARING_OPERATORS[] = {LT,LTE,EQ,GT,GTE};
const int COMPARING_OPERATORS_COUNT = 5;

int gErrorColumn = -1;
int gErrorCode = 0;

int gPoolSize = 0;
NMAST* AST_POOL[POOL_CAPACITY];

#ifdef DEBUG
int gNumberOfDynamicObject = 0;
int numberOfDynamicObject(){
	return gNumberOfDynamicObject;
}
void incNumberOfDynamicObject(){
	gNumberOfDynamicObject++;
}
void descNumberOfDynamicObject(){
	gNumberOfDynamicObject--;
}
void descNumberOfDynamicObjectBy(int k){
	gNumberOfDynamicObject -= k;
}
#endif

/** internal use */
void pushASTStack(NMASTList *sk, NMAST* ele) {
	NMAST** tmpP;
#ifdef DEBUG
	char isFirtTime = (sk==NULL || sk->loggedSize<=0)?TRUE:FALSE;
#endif

	if(sk == NULL)
		return;
		
	if(sk->size >= sk->loggedSize){
		sk->loggedSize += INCLEN;
		/**
			IMPORTANT: It may not secure here, if the realloc is failed it will return NULL
			and assign back to sk->list and finaly, the sk->list will be NULL accidentally
		*/
		tmpP = (NMAST**)realloc(sk->list, sizeof(NMAST*) * sk->loggedSize);
		if(tmpP == NULL){
			gErrorCode = E_NOT_ENOUGH_MEMORY;
			return;
		}
		sk->list = tmpP;
#ifdef DEBUG
		if(isFirtTime)
			incNumberOfDynamicObject();
#endif
		//if(lst != NULL)
		//	sk->list = lst;
	}
	sk->list[sk->size] = ele;
	(sk->size)++;
}

/** internal use */
NMAST* popASTStack(NMASTList *sk){
	NMAST* ele;
	if(sk == NULL || sk->size == 0)
		return NULL;
	ele = sk->list[sk->size-1];
	(sk->size)--;
	return ele;
}


int isPrime(long n){
	long i, sq;
	if(n<2) return 0;
	if(n==2) return 1;
	sq = (long) sqrt((double)n);
	for(i=2;i<=sq;i++)
		if( (n%i) == 0)
			return 0;
	return 1;
}

/* Greatest Common Divisor*/
long gcd(long a, long b){
	long c;
	while(a !=0 ){
		c = a;
		a = b % a;
		b = c;
	}
	return b;
}

long lcm(long a, long b){
	return (a*b)/gcd(a, b);
}

double parseDouble(const char *str, int start, int end, int *error){
	int i;
	double val = 0;
	char isFloatingPoint = 0;
	long floating = 1;
	char negative = 1;

	*error = -1;
	if(str == NULL)
		return 0;
		
	if(str[start] == '-'){
		negative = -1;
		start++;
	}

	for(i=start; i<end; i++){

		if(str[i]=='\0')
			return 0;

		if((str[i]<48) || (str[i]>57)){
			if( str[i] == 46 && isFloatingPoint==0)
				isFloatingPoint = 1;
			else{
				*error = ERROR_PARSE;
				/*printf(" Floating point ERROR F\n");*/
				return 0;
			}
		}else{
			if(isFloatingPoint){
				floating *= 10;
				val = val + (double)(str[i] - 48)/floating;
			}else
				val = val * 10 + (str[i] - 48);
		}
	}
	(*error) = 0;
	return val*negative;
}

void clearTree(NMAST **prf){
	
	if((*prf) == NULL)
		return;
	
	if((*prf)->left != NULL)
		clearTree(&((*prf)->left));
	if((*prf)->right != NULL)
		clearTree(&((*prf)->right));
		
	free(*prf);
	(*prf) = NULL;
	
#ifdef DEBUG
	descNumberOfDynamicObject();
#endif
	
}

void clearTreeContent(NMAST *prf){
	if(prf == NULL)
		return;
	
	if(prf->left != NULL){
		clearTreeContent(prf->left);
		free(prf->left);
		prf->left = NULL;
	}

	if(prf->right != NULL){
		clearTreeContent(prf->right);
		free(prf->right);
		prf->right = NULL;
	}
}

long l_cast(double val, double *fr){
	(*fr) = val - (long)val;
	return (long)val;
}

int l_castF(float val, float *fr){
	(*fr) = val - (int)val;
	return (int)val;
}

int contains(int type, const int *aset, int len){
	int i;
	for(i=0; i<len; i++)
		if(type == aset[i])
			return TRUE;
	return FALSE;
}

double logab(double a, double b, int *error){
	(*error) = 0;
	if( (b > 0.0) && (a > 0.0) && (a != 1.0))
		return log(b)/log(a);

	(*error) = ERROR_LOG;
	return 0;
}


float logabF(float a, float b, int *error){
	(*error) = 0;
	if( (b > 0.0f) && (a > 0.0f) && (a != 1.0f))
		return log(b)/log(a);

	(*error) = ERROR_LOG;
	return 0;
}


char getPriorityOfType(int type) {
	switch(type){
		case OR:
			return (char)1;
		break;
		
		case AND:
			return (char)2;
		break;
		
		case LT:
		case GT:
		case LTE:
		case GTE:
			return (char)3;
		break;
		
		case PLUS:
		case MINUS:
			return (char)4;
		break;
		
		case MULTIPLY:
		case DIVIDE:
			return (char)5;
		break;
		
		case POWER:
			return (char)6;
		break;
		
		case NE:
			return (char)7;
		break;
		
		default:
			return (char)0;
	}
}

double doCalculate(double val1, double val2, int type, int *error) {
	(*error) = 0;
	switch(type){
		case PLUS:
			/*printf("%lf+%lf=%lf\n", val1, val2, val1 + val2);*/
			return val1 + val2;

		case MINUS:
			/*printf("%lf-%lf=%lf\n", val1, val2, val1 - val2);*/
			return val1 - val2;

		case MULTIPLY:
			/*printf("%lf*%lf=%lf\n", val1, val2, val1 * val2);*/
			return val1 * val2;

		case DIVIDE:
			if(val2 == 0.0){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return val1/val2;

		case POWER:
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

float doCalculateF(float val1, float val2, int type, int *error) {
	(*error) = 0;
	switch(type){
		case PLUS:
			/*printf("%lf+%lf=%lf\n", val1, val2, val1 + val2);*/
			return val1 + val2;

		case MINUS:
			/*printf("%lf-%lf=%lf\n", val1, val2, val1 - val2);*/
			return val1 - val2;

		case MULTIPLY:
			/*printf("%lf*%lf=%lf\n", val1, val2, val1 * val2);*/
			return val1 * val2;

		case DIVIDE:
			if(val2 == 0.0f){
				(*error) = ERROR_DIV_BY_ZERO;
				return 0;
			}
			return val1/val2;

		case POWER:
			return pow(val1, val2);

		case LOG:
			return logabF(val1, val2, error);

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

int getErrorColumn(){
	return gErrorColumn;
}

int getErrorCode(){
	return gErrorCode;
}

int isAFunctionType(int type){
	int i;
	for(i=0; i<FUNCTION_COUNT; i++)
		if(type == FUNCTIONS[i])
			return TRUE;
	return FALSE;
}

int isAnOperatorType(int type){
	int i;
	for(i=0; i<OPERATOR_COUNT; i++)
		if(type == OPERATORS[i])
			return TRUE;
	return FALSE;
}

int isFunctionOROperator(int type){
	int i;
	for(i=0; i<FUNCTION_COUNT; i++)
		if(type == FUNCTIONS[i])
			return TRUE;

	for(i=0; i<OPERATOR_COUNT; i++)
		if(type == OPERATORS[i])
			return TRUE;

	return FALSE;
}

int isComparationOperator(int type){
	int i;
	for(i=0; i<COMPARING_OPERATORS_COUNT; i++)
		if(type == COMPARING_OPERATORS[i])
			return TRUE;
	return FALSE;
}

int isLetter(char c){
	if (( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ))
		return TRUE;
	return FALSE;
}

int isConstant(int type){
	if(type == NUMBER || type == PI_TYPE || type == E_TYPE)
		return TRUE;
	return FALSE;
}


/** Pool operations */
NMAST* getFromPool() {
	NMAST *node;
	if( gPoolSize>0 ) {
		node = AST_POOL[gPoolSize-1];
		AST_POOL[gPoolSize-1] = NULL;
		gPoolSize--;
	} else {
		node = (NMAST*)malloc(sizeof(NMAST));
		node->valueType = TYPE_FLOATING_POINT;
		node->sign = 1;
		node->variable = 0;
		node->left = node->right = node->parent = NULL;
		node->value = 0;
		node->type = NUMBER;
		node->priority = 0;
		//node->frValue.numerator = 0;
		//node->frValue.denomerator = 1;
#ifdef DEBUG
	gNumberOfDynamicObject++;
#endif
	}

	return node;
}

void putIntoPool(NMAST *node) {
	NMAST *p;

	if(node == NULL) return;

	if(node->left != NULL)
		putIntoPool(node->left);

	if(node->right != NULL)
		putIntoPool(node->right);

	if(node->parent != NULL) {
		p = node->parent;
		if(p->left == node)
			p->left = NULL;
		else if(p->right == node)
			p->right = NULL;

		node->parent = NULL;
	}

	node->valueType = TYPE_FLOATING_POINT;
	node->sign = 1;
	node->variable = 0;
	node->left = node->right = node->parent = NULL;
	node->value = 0;
	node->type = NUMBER;
	node->priority = 0;
	//node->frValue.numerator = 0;
	//node->frValue.denomerator = 1;

	if( gPoolSize < POOL_CAPACITY ) {
		AST_POOL[gPoolSize] = node;
		gPoolSize++;
		return;
	}

	free(node);
}

void clearPool() {
	int i;
	if(gPoolSize > 0) {
		for(i=0; i<gPoolSize; i++) {
			free(AST_POOL[i]);
			AST_POOL[i] = NULL;
		}
		gPoolSize = 0;
	}
}

std::ostream& printNMAST(const NMAST *ast, int level, std::ostream& os) {
	int i;

	if (ast == NULL) return os;

	if (level > 0){
		for (i = 0; i<level - 1; i++)
			os << "\t";
		os << "|--";
	}

	switch (ast->type) {
	case AND:
		os << "AND \n";
		break;

	case OR:
		os << "OR \n";
		break;

	case LT:
		os << "< \n";
		break;

	case LTE:
		os << "(variable:" << ast->variable << " <= value :" << ast->value << ")\n";
		break;

	case GT:
		printf("(variable:%c > value:%lf) \n", ast->variable, ast->value);
		break;

	case NAME:
	case VARIABLE:
		os << ast->variable << "\n";
		break;

	case NUMBER:
		os << ast->value << "\n";
		break;

	case PI_TYPE:
		printf("PI \n");
		break;

	case E_TYPE:
		printf("e \n");
		break;

	case PLUS:
		printf("+ \n");
		break;

	case MINUS:
		printf("- \n");
		break;

	case MULTIPLY:
		printf("* \n");
		break;

	case DIVIDE:
		os << "/ \n";
		break;

	case POWER:
		os << "^";
		break;

	default:
		printf("(type=%d) \n", ast->type);
	}

	if (ast->left != NULL)
		printNMAST(ast->left, level + 1, os);

	if (ast->right != NULL)
		printNMAST(ast->right, level + 1, os);

	return os;
}
