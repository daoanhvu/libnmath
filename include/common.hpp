#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "internal_common.h"


namespace nmath {
    /* ====================================================================================================== */
    /**
     * This do the primity test
     * return 1: if n is prime
     * otherwise return 0 */
    template <typename T>
    bool isPrime(T n){
        T sq;
        if(n<(T)2) return (T)0;
        if(n==(T)2) return (T)1;
        sq = (T)sqrt((double)n);
        for(T i=2;i <= sq;i++)
            if( (n % i) == 0)
                return (T)0;
        return (T)1;
    }

    /** Greatest Common Divisor*/
    template <typename T>
    T gcd(T a, T b){
        T c;
        while(a !=0 ){
            c = a;
            a = b % a;
            b = c;
        }
        return b;
    }

    template <typename T>
    T lcm(T a, T b){
        return (a*b)/gcd<T>(a, b);
    }

    template <typename T>
    T parseDouble(const char *str, int start, int end, int *error) {
        T val = 0;
        char isFloatingPoint = 0;
        long floating = 1;
        char negative = 1;

        *error = -1;
        if(str == nullptr)
            return 0;

        if(str[start] == '-'){
            negative = -1;
            start++;
        }

        for(int i=start; i<end; i++) {

            // TODO: Fix this
            if(str[i]=='\0')
                return (T)0;

            if((str[i]<48) || (str[i]>57)) {
                if( str[i] == 46 && isFloatingPoint==0)
                    isFloatingPoint = 1;
                else{
                    *error = ERROR_PARSE;
                    /*printf(" Floating point ERROR F\n");*/
                    return (T)0;
                }
            } else {
                if(isFloatingPoint){
                    floating *= 10;
                    val = val + (T)(str[i] - 48)/floating;
                } else {
                    val = val * 10 + (str[i] - 48);
                }
            }
        }
        (*error) = 0;
        return val*negative;
    }

    template <typename T>
    T parseInteger(const char *str, int start, int end, int *error) {
        T val = (T)0;
        const char C_48 = 48;
        const char C_57 = 57;
        *error = -1;
        if(str == nullptr)
            return 0;

        int negative = 1;
        if(str[start] == '-') {
            negative = -1;
            start++;
        }
        for(auto i=start; i<end; i++) {
            if((str[i] >= C_48) && (str[i]<=C_57)) {
                val = val * (T)10 + (T)(str[i] - C_48);
            } else {
                *error = i;
                return val;
            }
        }
        return val;
    }

    template <typename T>
    void clearTree(NMAST<T> **prf){
        if((*prf) == nullptr)
            return;
        if((*prf)->left != nullptr)
            nmath::clearTree(&((*prf)->left));
        if((*prf)->right != nullptr)
            nmath::clearTree(&((*prf)->right));
        delete (*prf);
        (*prf) = nullptr;
    }

    template <typename T>
    void clearTreeContent(NMAST<T> *prf){
        if(prf == nullptr)
            return;

        if(prf->left != nullptr){
            clearTreeContent(prf->left);
            delete (prf->left);
            prf->left = nullptr;
        }

        if(prf->right != nullptr){
            clearTreeContent(prf->right);
            delete (prf->right);
            prf->right = nullptr;
        }
    }
    /**
    * Extract the decimal part from a floating point
    * @param val a value of FT {floating-point type such as float, double}
    * @param [OUT] fr the value after extracting
    * @return a integer represent the value of fr
    */
    template <typename IT, typename FT>
    IT l_cast(FT val, FT *fr){
        (*fr) = val - (IT)val;
        return (IT)val;
    }

    template <typename T>
    bool contains(T aValue, const T *aset, unsigned int size){
        for(unsigned int i=0; i<size; i++)
            if(aValue == aset[i])
                return true;
        return false;
    }

    template <typename T>
    T logab(T a, T b, int *error){
        (*error) = 0;
        if( (b > 0.0) && (a > (T)0) && (a != (T)1))
            return log(b)/log(a);

        (*error) = ERROR_LOG;
        return 0;
    }

    template <typename T>
    T doCalculate(T val1, T val2, int type, int *error) {
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

            case ABS:
                return (val2<0)?(-val2):val2;

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

            default:
                return 0;
        }
    }

    template <typename IT, typename FT>
    void toString(const NMAST<FT> *t, char *str, int *curpos, int len) {
        FT fr;
        IT lval;
        int i, l;
        char operatorChar = 0;
        char tmp[16];
        bool parentHasHigherPriority;

        if (t == nullptr)
            return;

        if(t->sign < 0) {
            str[(*curpos)] = '-';
            (*curpos)++;
        }

        switch (t->type){
            case NUMBER:
                lval = l_cast<IT, FT>(t->value, &fr);
                if (fr != 0.0){
                    l = sprintf(tmp, "%lf", t->value);
                } else {
                    l = sprintf(tmp, "%lf", lval);
                }

                for (i = 0; i<l; i++) {
                    str[(*curpos)] = tmp[i];
                    (*curpos)++;
                }
                break;

            case PI_TYPE:
                str[(*curpos)] = 'P';
                str[(*curpos) + 1] = 'I';
                (*curpos) += 2;
                break;

            case E_TYPE:
                str[(*curpos)] = 'e';
                (*curpos)++;
                break;

            case VARIABLE:
            case NAME:
                strcpy(str + (*curpos), t->text.c_str());
                (*curpos) += t->text.length();
                break;

            case PLUS:
            case MINUS:
            case MULTIPLY:
            case DIVIDE:
            case POWER:
                parentHasHigherPriority = (t->parent != nullptr) && ((t->parent)->priority > t->priority);
                getOperatorChar(t->type, &operatorChar);
                if (parentHasHigherPriority) {
                    str[(*curpos)] = '(';
                    (*curpos)++;
                }

                if (t->left != nullptr)
                    toString<IT,FT>(t->left, str, curpos, len);

                str[(*curpos)] = operatorChar;
                (*curpos)++;

                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);

                if (parentHasHigherPriority) {
                    str[(*curpos)] = ')';
                    (*curpos)++;
                }
                break;

            case SIN:
                //printf("sin(");
                str[(*curpos)] = 's';
                str[(*curpos) + 1] = 'i';
                str[(*curpos) + 2] = 'n';
                str[(*curpos) + 3] = '(';
                (*curpos) += 4;

                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);
                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case ASIN:
                //printf("asin(");
                str[(*curpos)] = 'a';
                str[(*curpos) + 1] = 's';
                str[(*curpos) + 2] = 'i';
                str[(*curpos) + 3] = 'n';
                str[(*curpos) + 4] = '(';
                (*curpos) += 5;

                if (t->right != nullptr)
                    toString<IT, FT>(t->right, str, curpos, len);

                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case COS:
                //printf("cos(");
                str[(*curpos)] = 'c';
                str[(*curpos) + 1] = 'o';
                str[(*curpos) + 2] = 's';
                str[(*curpos) + 3] = '(';
                (*curpos) += 4;

                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);

                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case ACOS:
                //printf("acos(");
                str[(*curpos)] = 'a';
                str[(*curpos) + 1] = 'c';
                str[(*curpos) + 2] = 'o';
                str[(*curpos) + 3] = 's';
                str[(*curpos) + 4] = '(';
                (*curpos) += 5;

                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);

                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case ATAN:
                //printf("atan(");
                str[(*curpos)] = 'a';
                str[(*curpos) + 1] = 't';
                str[(*curpos) + 2] = 'a';
                str[(*curpos) + 3] = 'n';
                str[(*curpos) + 4] = '(';
                (*curpos) += 5;

                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);

                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case LN:
                //printf("ln(");
                str[(*curpos)] = 'l';
                str[(*curpos) + 1] = 'n';
                str[(*curpos) + 2] = '(';
                (*curpos) += 3;

                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);

                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case LOG:
                //printf("log(");
                str[(*curpos)] = 'l';
                str[(*curpos) + 1] = 'o';
                str[(*curpos) + 2] = 'g';
                str[(*curpos) + 3] = '(';
                (*curpos) += 4;
                if (t->left != nullptr)
                    toString<IT,FT>(t->left, str, curpos, len);
                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);
                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case SQRT:
                //printf("sqrt(");
                str[(*curpos)] = 's';
                str[(*curpos) + 1] = 'q';
                str[(*curpos) + 2] = 'r';
                str[(*curpos) + 3] = 't';
                str[(*curpos) + 4] = '(';
                (*curpos) += 5;
                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);
                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case SEC:
                //printf("sec(");
                str[(*curpos)] = 's';
                str[(*curpos) + 1] = 'e';
                str[(*curpos) + 2] = 'c';
                str[(*curpos) + 3] = '(';
                (*curpos) += 4;
                if (t->right != nullptr)
                    toString<IT,FT>(t->right, str, curpos, len);
                str[(*curpos)] = ')';
                (*curpos)++;
                break;

            case AND:
                str[(*curpos)] = 'A';
                str[(*curpos) + 1] = 'N';
                str[(*curpos) + 2] = 'D';
                (*curpos) += 3;
                break;

            case OR:
                str[(*curpos)] = 'O';
                str[(*curpos) + 1] = 'R';
                (*curpos) += 2;
                break;

            default:
                break;
        }
    }

    /*****************************************************************************************************************/
    template <typename T>
    void releaseNMATree(std::vector<NMAST<T>*> &t) {
        if (t.size() <= 0) return;
        for (unsigned long i = 0; i<t.size(); i++){
            clearTree(&(t[i]));
        }
        t.clear();
    }


    /**
        Check if a tree contains variable x
        @param t the tree
        @param x variable to check if existed
    */
    template <typename T>
    bool isContainVar(NMAST<T> *t, std::string x){

        if ((t == nullptr) || (t->type == NUMBER) || (t->type == PI_TYPE) || (t->type == E_TYPE))
            return false;

        if (t->type == VARIABLE) {
            if (t->text == x)
                return (t->sign>0);
            return false;
        }

        return (isContainVar(t->left, x) || isContainVar(t->right, x));
    }

    template <typename T>
    NMAST<T>* cloneTree(NMAST<T> *t, NMAST<T> *cloneParent){
        NMAST<T> *c;

        if(t==nullptr) {
            return nullptr;
        }

        c = new NMAST<T>;
        c->text = t->text;
        c->type = t->type;
        c->value = t->value;
        c->priority = t->priority;
        c->level = t->level;
        c->sign = t->sign;

        c->parent = cloneParent;
        c->left = cloneTree(t->left, c);
        c->right = cloneTree(t->right, c);
        return c;
    }

// #ifdef _PCDEBUG
    template <typename T>
    std::ostream& printNMAST(const NMAST<T> *ast, int level, std::ostream& os) {
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
            os << "(variable:" << ast->text << " < value :" << ast->value << ")\n";
            break;

        case LTE:
            os << "(variable:" << ast->text << " <= value :" << ast->value << ")\n";
            break;

        case GT:
            os << "(variable:"<< ast->text <<" > value:"<< ast->value <<") \n";
            break;

        case NAME:
        case VARIABLE:
            os << ast->text << "\n";
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
            printNMAST<T>(ast->left, level + 1, os);

        if (ast->right != NULL)
            printNMAST<T>(ast->right, level + 1, os);

        return os;
    }
    // #endif //end _PCDEBUG

} //end namespace nmath

#endif
