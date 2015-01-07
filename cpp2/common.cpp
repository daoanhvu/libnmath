#ifdef _WIN32
//#include <Windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "common.h"

#define POOL_CAPACITY 32

#ifdef _ADEBUG
#include <android/log.h>
#define LOG_TAG "NMATH_COMMON"
#define LOG_LEVEL 10
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
#endif

using namespace nmath;

const int FUNCTIONS[] = {SIN, COS, TAN, COTAN, ASIN, ACOS, ATAN, LOG, LN, SQRT};
const int FUNCTION_COUNT = 10;

const int OPERATORS[] = {PLUS,MINUS,MULTIPLY,DIVIDE,POWER};
const int OPERATOR_COUNT = 5;

const int COMPARING_OPERATORS[] = {LT,LTE,EQ,GT,GTE};
const int COMPARING_OPERATORS_COUNT = 5;

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
int nmath::pushASTStack(NMASTList *sk, NMAST* ele) {
	NMAST** tmpP;

	if(sk == NULL)
		return 0;
		
	if(sk->size >= sk->loggedSize){
		sk->loggedSize += INCLEN;
		/**
			IMPORTANT: It may not secure here, if the realloc is failed it will return NULL
			and assign back to sk->list and finaly, the sk->list will be NULL accidentally
		*/
		tmpP = (NMAST**)realloc(sk->list, sizeof(NMAST*) * sk->loggedSize);
		if(tmpP == NULL){
			return E_NOT_ENOUGH_MEMORY;
		}
		sk->list = tmpP;
		//if(lst != NULL)
		//	sk->list = lst;
	}
	sk->list[sk->size] = ele;
	(sk->size)++;

	return NMATH_NO_ERROR;
}

/** internal use */
NMAST* nmath::popASTStack(NMASTList *sk){
	NMAST* ele;
	if(sk == NULL || sk->size == 0)
		return NULL;
	ele = sk->list[sk->size-1];
	(sk->size)--;
	return ele;
}


int nmath::isPrime(long n){
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
long nmath::gcd(long a, long b){
	long c;
	while(a !=0 ){
		c = a;
		a = b % a;
		b = c;
	}
	return b;
}

long nmath::lcm(long a, long b){
	return (a*b)/nmath::gcd(a, b);
}

double nmath::parseDouble(const char *str, int start, int end, int *error){
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

void nmath::clearTree(NMAST **prf){
	
	if((*prf) == NULL)
		return;
	
	if((*prf)->left != NULL)
		nmath::clearTree(&((*prf)->left));
	if((*prf)->right != NULL)
		nmath::clearTree(&((*prf)->right));
		
	free(*prf);
	(*prf) = NULL;
}

void nmath::clearTreeContent(NMAST *prf){
	if(prf == NULL)
		return;
	
	if(prf->left != NULL){
		nmath::clearTreeContent(prf->left);
		free(prf->left);
		prf->left = NULL;
	}

	if(prf->right != NULL){
		nmath::clearTreeContent(prf->right);
		free(prf->right);
		prf->right = NULL;
	}
}

long nmath::l_cast(double val, double *fr){
	(*fr) = val - (long)val;
	return (long)val;
}

int nmath::l_castF(float val, float *fr){
	(*fr) = val - (int)val;
	return (int)val;
}

int nmath::contains(int type, const int *aset, int len){
	int i;
	for(i=0; i<len; i++)
		if(type == aset[i])
			return TRUE;
	return FALSE;
}

double nmath::logab(double a, double b, int *error){
	(*error) = 0;
	if( (b > 0.0) && (a > 0.0) && (a != 1.0))
		return log(b)/log(a);

	(*error) = ERROR_LOG;
	return 0;
}


float nmath::logabF(float a, float b, int *error){
	(*error) = 0;
	if( (b > 0.0f) && (a > 0.0f) && (a != 1.0f))
		return log(b)/log(a);

	(*error) = ERROR_LOG;
	return 0;
}


char nmath::getPriorityOfType(int type) {
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

double nmath::doCalculate(double val1, double val2, int type, int *error) {
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

float nmath::doCalculateF(float val1, float val2, int type, int *error) {
	(*error) = 0;
#ifdef _ADEBUG
	float f;
#endif
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

int nmath::isAFunctionType(int type){
	int i;
	for(i=0; i<FUNCTION_COUNT; i++)
		if(type == FUNCTIONS[i])
			return TRUE;
	return FALSE;
}

int nmath::isAnOperatorType(int type){
	int i;
	for(i=0; i<OPERATOR_COUNT; i++)
		if(type == OPERATORS[i])
			return TRUE;
	return FALSE;
}

int nmath::isFunctionOROperator(int type){
	int i;
	for(i=0; i<FUNCTION_COUNT; i++)
		if(type == FUNCTIONS[i])
			return TRUE;

	for(i=0; i<OPERATOR_COUNT; i++)
		if(type == OPERATORS[i])
			return TRUE;

	return FALSE;
}

int nmath::isComparationOperator(int type){
	int i;
	for(i=0; i<COMPARING_OPERATORS_COUNT; i++)
		if(type == COMPARING_OPERATORS[i])
			return TRUE;
	return FALSE;
}

int nmath::isLetter(char c){
	if (( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ))
		return TRUE;
	return FALSE;
}

int nmath::isConstant(int type){
	if(type == NUMBER || type == PI_TYPE || type == E_TYPE)
		return TRUE;
	return FALSE;
}

#ifdef _DEBUG
std::ostream& nmath::printNMAST(const NMAST *ast, int level, std::ostream& os) {
	int i;

	if (ast == NULL) return os;

	if (level > 0){
		for (i = 0; i<level; i++)
			os << " ";
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
		os << "(variable:" << ast->variable << " < value :" << ast->value << ")\n";
		break;

	case LTE:
		os << "(variable:" << ast->variable << " <= value :" << ast->value << ")\n";
		break;

	case GT:
		os << "(variable:"<< ast->variable <<" > value:"<< ast->value <<") \n";
		break;

	case NAME:
	case VARIABLE:
		os << ast->variable << "\n";
		break;

	case NUMBER:
		os << ast->value << "\n";
		break;

	case PI_TYPE:
		os << "PI \n";
		break;

	case E_TYPE:
		os << "e \n";
		break;

	case PLUS:
		os << "+ \n";
		break;

	case MINUS:
		os << "- \n";
		break;

	case MULTIPLY:
		os <<"* \n";
		break;

	case DIVIDE:
		os << "/ \n";
		break;

	case POWER:
		os << "^ \n";
		break;

	case SIN:
		os << "SIN \n";
		break;

	case COS:
		os << "COS \n";
		break;

	default:
		os << "(type=" << ast->type << ") \n";
	}

	if (ast->left != NULL)
		printNMAST(ast->left, level + 1, os);

	if (ast->right != NULL)
		printNMAST(ast->right, level + 1, os);

	return os;
}
#endif