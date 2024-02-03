#ifndef _CALCULUS_H
#define _CALCULUS_H

#include <string>
#include "common.hpp"
#include "common_data.h"
#include "nmath_pool.hpp"

using namespace std;

namespace nmath {

    template <typename T>
    NMASTPool<T> gPool;

    template <typename T>
    void clearPool() {
        gPool.clear();
    }

    template <typename T>
    NMAST<T>* cloneTree(NMAST<T> *t, NMAST<T> *cloneParent);
    template <typename T>
    NMAST<T>* d_product(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_sin(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_cos(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_tan(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_cotan(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_asin(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_acos(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_atan(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_sqrt(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_pow_exp(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_sum_subtract(NMAST<T> *t, int type, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_quotient(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    NMAST<T>* d_ln(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x);
    template <typename T>
    bool isContainVar(NMAST<T> *t, string x);

    template <typename T> int reduce_plus(NMAST<T> *t);
    template <typename T> int reduce_multiply(NMAST<T> *t);
    template <typename T> int reduce_divide(NMAST<T> *t);
    template <typename T> int reduce_power(NMAST<T> *t);
	template <typename T> int calculate_function(NMAST<T>* dp);
    template <typename T> int reduce_t(DParam<T> *dp);
    template <typename T> int calc_t(DParam<T> *dp);
    template <typename T> int derivative(DParam<T> *dp);


template <typename T>
int calculate_function(NMAST<T>* t) {

    if(!isConstant(t->right->type)) {
        return NMATH_NO_ERROR;
    }

    NMAST<T> *p;
    int error;
    t->value = doCalculate<T>(0, t->right->value, t->type, &error);
    if (error != 0) {
        return error;
    }
    t->type = NUMBER;
    if (t->left != nullptr){
        p = t->left;
        t->left = nullptr;
        delete p;
    }
    p = t->right;
    t->right = nullptr;
    delete p;
    return error;
}

template <typename T>
int reduce_plus(NMAST<T> *t) {
    NMAST<T> *p;

    if ((t->left == nullptr) || (t->right == nullptr) ) {
        return ERROR_OPERAND_MISSING;
    }

    /* take care of special cases */
    if (t->type == PLUS) {
        /* 0 + something */
        if ((t->left->type == NUMBER) && ( t->left->value == (T)0) ) {

            //remove the left child (value 0)
            p = t->left;
            p->parent = nullptr;
            t->left = nullptr;
            delete p;

            //move right child to its parent
            p = t->right;
            t->type = p->type;
            t->text = p->text;
            t->value = p->value;
            t->priority = p->priority;
            t->sign = p->sign;
            /* NO copy parent */
            t->left = p->left;
            t->right = p->right;
            /* Now release p */
            delete p;
            return NMATH_NO_ERROR;
        }

        /* something + 0 */
        if ((t->right->type == NUMBER) && (t->right->value == (T)0)) {

            //remove the right child (value 0)
            p = t->right;
            t->right = nullptr;
            delete p;

            p = t->left;
            t->type = p->type;
            t->text = p->text;
            t->value = p->value;
            t->priority = p->priority;
            t->sign = p->sign;
            /* NO copy parent */
            t->left = p->left;
            t->right = p->right;
            /* Now release p */
            delete p;
            return NMATH_NO_ERROR;
        }
    } else {

        if (t->type == MINUS) {
            /* 0 - something */
            if ((t->left->type == NUMBER) && (t->left->value == (T) 0)) {

                //remove the left child (value 0)
                p = t->left;
                p->parent = nullptr;
                t->left = nullptr;
                delete p;

                //move right child to its parent
                p = t->right;
                t->type = p->type;
                t->text = p->text;
                t->value = p->value;
                t->priority = p->priority;
                t->sign = -(p->sign);
                /* NO copy parent */
                t->left = p->left;
                t->right = p->right;
                /* Now release p */
                delete p;
                return NMATH_NO_ERROR;
            }

            /* something - 0 */
            if ((t->right->type == NUMBER) && (t->right->value == (T) 0)) {

                //remove the right child (value 0)
                p = t->right;
                t->right = nullptr;
                delete p;

                p = t->left;
                t->type = p->type;
                t->text = p->text;
                t->value = p->value;
                t->priority = p->priority;
                t->sign = p->sign;
                /* NO copy parent */
                t->left = p->left;
                t->right = p->right;
                /* Now release p */
                delete p;
                return NMATH_NO_ERROR;
            }
        }
    }

    // trivial cases
    int error;
    if (isConstant((t->left)->type) && isConstant((t->right)->type)) {
        t->value = doCalculate((t->left)->value, (t->right)->value, t->type, &error);
        if (error != 0) {
            return error;
        }
        p = t->left;
        p->parent = nullptr;
        delete p;
        p = t->right;
        p->parent = nullptr;
        delete p;
        t->type = NUMBER;
        t->priority = 0;
        t->left = t->right = nullptr;
        return NMATH_NO_ERROR;
    }

    return NMATH_NO_ERROR;
}

template <typename T>
int reduce_multiply(NMAST<T> *t) {
    NMAST<T> *p;
    if ((t->left == nullptr) || (t->right == nullptr) ) {
        return ERROR_OPERAND_MISSING;
    }

    /* (0 * something) OR (something * 0) */
    if (((t->left)->type == NUMBER && (t->left)->value == (T)0) ||
        ((t->right)->type == NUMBER && (t->right)->value == (T)0)){

        clearTree(&(t->left));
        clearTree(&(t->right));
        t->left = nullptr;
        t->right = nullptr;
        t->type = NUMBER;
        t->value = 0;
        t->priority = 0;
        t->sign = 1;
        t->text = "0";
        return NMATH_NO_ERROR;
    }

    /* (1 * something) */
    if (((t->left)->type == NUMBER && (t->left)->value == (T)1)) {
        clearTree(&(t->left));
        p = t->right;
        p->parent = nullptr;
        t->type = p->type;
        t->value = p->value;
        t->sign = p->sign;
        t->text = p->text;
        t->priority = p->priority;
        t->left = p->left;
        t->right = p->right;
        delete p;
        return NMATH_NO_ERROR;
    }

    /* (something * 1) */
    if (((t->right)->type == NUMBER && (t->right)->value == (T)1)){
        clearTree(&(t->right));
        p = t->left;
        p->parent = nullptr;
        t->type = p->type;
        t->value = p->value;
        t->sign = p->sign;
        t->text = p->text;
        t->priority = p->priority;
        t->left = p->left;
        t->right = p->right;
        delete p;
        return NMATH_NO_ERROR;
    }
    return NMATH_NO_ERROR;
}

template <typename T>
int reduce_divide(NMAST<T> *t) {
    NMAST<T> *p;

    if ((t->left == nullptr) || (t->right == nullptr) ) {
        return ERROR_OPERAND_MISSING;
    }

    // something / 1
    if ((t->right->type == NUMBER) && (t->right->value == (T)1)) {
        p = t->right;
        p->parent = nullptr;
        delete p;
        p = t->left;
        t->type = p->type;
        t->text = p->text;
        t->value = p->value;
        t->priority = p->priority;
        t->sign = p->sign;
        t->left = p->left;
        t->right = p->right;
        delete p;
        return NMATH_NO_ERROR;
    }

    /* 0 / something */
    if ( (t->left->type == NUMBER) && (t->left->value == (T)0)) {
        return NMATH_NO_ERROR;
    }

    /* Error case: something / 0 */
    if ((t->right->type == NUMBER) && (t->right->value == (T)0)) {
        return ERROR_DIV_BY_ZERO;
    }

    return NMATH_NO_ERROR;
}

template <typename T>
int reduce_power(NMAST<T> *t) {
    NMAST<T> *p;

    if ((t->left == nullptr) || (t->right == nullptr) ) {
        return ERROR_OPERAND_MISSING;
    }

    if ((t->right)->type == NUMBER) {
        if ((t->right)->value == (T)0) {
            // something ^ 0
            clearTree<T>(&(t->left));
            clearTree<T>(&(t->right));
            t->left = nullptr;
            t->right = nullptr;
            t->type = NUMBER;
            t->text = "1";
            t->value = (T)1;
            t->priority = 0;
            t->sign = 1;
            return NMATH_NO_ERROR;
        }

        if ((t->right)->value == (T)1) {
            // something ^ 1
            p = t->right;
            p->parent = nullptr;
            delete p;
            p = t->left;
            t->type = p->type;
            t->text = p->text;
            t->value = p->value;
            t->priority = p->priority;
            t->sign = p->sign;
            t->left = p->left;
            t->right = p->right;
            delete p;
            return NMATH_NO_ERROR;
        }
    }

    // 0 ^ something
    if (((t->left)->type == NUMBER) && ((t->left)->value == (T)0)) {
        p = t->right;
        p->parent = nullptr;
        delete p;
        p = t->left;
        p->parent = nullptr;
        delete p;
        t->type = NUMBER;
        t->value = (T)0;
        t->text = "0";
        t->priority = 0;
        t->sign = p->sign;
        t->left = t->right = nullptr;
        return NMATH_NO_ERROR;
    }

    // number ^ number, this is a trivial case
    if (isConstant(t->left->type) && isConstant(t->right->type)) {

        t->value = pow(t->left->value, t->right->value);
        t->type = NUMBER;
        t->priority = 0;
        t->sign = 1;
        p = t->left;
        p->parent = nullptr;
        delete p;
        p = t->right;
        p->parent = nullptr;
        delete p;
        t->left = t->right = nullptr;
        return NMATH_NO_ERROR;
    }

    return NMATH_NO_ERROR;
}

/* (u.v) = u'v + uv' */
template <typename T>
NMAST<T>* d_product(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r = nullptr;
    if (dv == nullptr && du != nullptr) {
        if(du->type == NUMBER && du->value == 0.0) {
            r = du;
        } else {
            r = new NMAST<T>;
            r->sign = 1;
            r->text = "*";
            r->type = MULTIPLY;
            r->parent = nullptr;
            r->left = du;
            du->parent = r;
            r->right = cloneTree(v, r);
        }
        return r;
    }

    if(dv != nullptr && du == nullptr) {
        if( (dv->type==NUMBER) && (dv->value == (T)0) ) {
            r = dv;
        } else {
            r = new NMAST<T>;
            r->sign = 1;
            r->text = "*";
            r->type = MULTIPLY;
            r->left = cloneTree(u, r);
            r->right = dv;
            dv->parent = r;
        }

        return r;
    }

    r = new NMAST<T>;
    r->sign = 1;
    r->text = "+";
    r->type = PLUS;
    r->parent = nullptr;


    if(dv->type == NUMBER && dv->value == 0.0) {
        r->left = dv;
        dv->parent = r;
    } else {
        r->left = new NMAST<T>;
        r->left->sign = 1;
        r->left->text = "*";
        r->left->type = MULTIPLY;
        r->left->parent = r;
        r->left->left = cloneTree(u, r->left);
        r->left->right = dv;
        dv->parent = r->left;
    }

    if(du->type==NUMBER && du->value == 0.0) {
        r->right = du;
        du->parent = r;
    }else {
        r->right = new NMAST<T>;
        r->right->sign = 1;
        r->right->text = "*";
        r->right->type = MULTIPLY;
        r->right->parent = r;
        r->right->left = du;
        du->parent = r->right;
        r->right->right = cloneTree(v, r->right);
    }

    return r;
}

/* (sin(v))' = cos(v)*dv */
template <typename T>
NMAST<T>* d_sin(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;
    /* (cos(v))' = -sin(v)*dv */
    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = COS;
    r->left->sign = 1;
    r->left->parent = r;
    r->left->left = nullptr;
    r->left->right = cloneTree(v, r);

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;

    return r;
}

/* (cos(v))' = -sin(v)dv */
template <typename T>
NMAST<T>* d_cos(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;
    /* (cos(v))' = -sin(v)*dv */
    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = SIN; /* <== negative here */
    r->left->sign = -1;
    r->left->parent = r;
    r->left->left = nullptr;
    r->left->right = cloneTree(v, r);

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;

    return r;
}

/* tan(v)' =  (sec(v)^2)*dv  */
template <typename T>
NMAST<T>* d_tan(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;

    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = POWER;
    r->left->sign = 1;
    r->left->parent = r;

    r->left->left = new NMAST<T>;
    r->left->left->parent = r->left;
    r->left->left->type = SEC;

    r->left->left->left = nullptr;
    r->left->left->right = cloneTree(v, r->left->left);

    r->left->right = new NMAST<T>;
    r->left->right->parent = r->left;
    r->left->right->type = NUMBER;
    r->left->right->value = 2;
    r->left->right->sign = 1;

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;

    return r;
}

/* cotan(v)' = -(1 -sqrt(cotan(v))) * dv  */
template <typename T>
NMAST<T>* d_cotan(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;

    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = PLUS;
    r->left->value = 1.0;
    r->left->sign = -1;
    r->left->parent = r;

    r->left->left = new NMAST<T>;
    r->left->left->type = NUMBER;
    r->left->left->value = 1.0;
    r->left->left->sign = 1;
    r->left->left->parent = r->left;

    r->left->right = new NMAST<T>;
    r->left->right->type = SQRT;
    r->left->right->value = 1.0;
    r->left->right->sign = 1;
    r->left->right->parent = r->left;

    r->left->right->left = new NMAST<T>;
    r->left->right->left->type = COTAN;
    r->left->right->left->value = 1.0;
    r->left->right->left->sign = 1;
    r->left->right->left->parent = r->left->right;

    r->left->right->left->left = cloneTree(v, r->left->right->left);

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;

    return r;
}

/* arcsin(v)' = (1/sqrt(1-v^2))*dv */
template <typename T>
NMAST<T>* d_asin(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;
    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = DIVIDE;
    r->left->sign = 1;
    r->left->parent = r;

    r->left->left = new NMAST<T>;
    r->left->left->type = NUMBER;
    r->left->left->value = 1;
    r->left->left->sign = 1;
    r->left->left->parent = r->left;
    r->left->left->left = r->left->left->right = nullptr;

    /* sqrt(...) */
    r->left->right = new NMAST<T>;
    r->left->right->type = SQRT;
    r->left->right->parent = r->left;
    r->left->right->left = nullptr;

    r->left->right->right = new NMAST<T>;
    r->left->right->right->type = MINUS;
    r->left->right->right->parent = r->left->right;

    r->left->right->right->left = new NMAST<T>;
    r->left->right->right->left->type = NUMBER;
    r->left->right->right->left->value = 1;
    r->left->right->right->left->sign = 1;
    r->left->right->right->left->parent = r->left->right->right;
    r->left->right->right->left->left = r->left->right->right->left->right = nullptr;

    r->left->right->right->right = new NMAST<T>;
    r->left->right->right->right->type = POWER;
    r->left->right->right->right->value = 0;
    r->left->right->right->right->sign = 1;
    r->left->right->right->right->parent = r->left->right->right;

    r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);

    r->left->right->right->right->right = new NMAST<T>;
    r->left->right->right->right->right->type = NUMBER;
    r->left->right->right->right->right->value = 2;
    r->left->right->right->right->right->sign = 1;
    r->left->right->right->right->right->parent = r->left->right->right->right;
    r->left->right->right->right->right->left = r->left->right->right->right->right->right = nullptr;

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;
    return r;
}

/* arccos(v)' = (-1/sqrt(1-v^2))*dv */
template <typename T>
NMAST<T>* d_acos(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;
    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = DIVIDE;
    r->left->sign = 1;
    r->left->parent = r;

    r->left->left = new NMAST<T>;
    r->left->left->type = NUMBER;
    r->left->left->value = -1;
    r->left->left->sign = 1;
    r->left->left->parent = r->left;
    r->left->left->left = r->left->left->right = nullptr;

    /* sqrt(...) */
    r->left->right = new NMAST<T>;
    r->left->right->sign = 1;
    r->left->right->type = SQRT;
    r->left->right->parent = r->left;
    r->left->right->left = nullptr;

    r->left->right->right = new NMAST<T>;
    r->left->right->right->sign = 1;
    r->left->right->right->type = MINUS;
    r->left->right->right->parent = r->left->right;

    r->left->right->right->left = new NMAST<T>;
    r->left->right->right->left->type = NUMBER;
    r->left->right->right->left->value = 1;
    r->left->right->right->left->sign = 1;
    r->left->right->right->left->parent = r->left->right->right;
    r->left->right->right->left->left = r->left->right->right->left->right = nullptr;

    r->left->right->right->right = new NMAST<T>;
    r->left->right->right->right->type = POWER;
    r->left->right->right->right->value = 0;
    r->left->right->right->right->sign = 1;
    r->left->right->right->right->parent = r->left->right->right;

    r->left->right->right->right->left = cloneTree(v, r->left->right->right->right);

    r->left->right->right->right->right = new NMAST<T>;
    r->left->right->right->right->right->type = NUMBER;
    r->left->right->right->right->right->value = 2;
    r->left->right->right->right->right->sign = 1;
    r->left->right->right->right->right->parent = r->left->right->right->right;
    r->left->right->right->right->right->left = r->left->right->right->right->right->right = nullptr;

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;
    return r;
}

/* arctan(v)' = (1/(v^2+1))*dv */
template <typename T>
NMAST<T>* d_atan(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;
    r = new NMAST<T>;
    r->type = MULTIPLY;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->type = DIVIDE;
    r->left->sign = 1;
    r->left->parent = r;

    r->left->left = new NMAST<T>;
    r->left->left->type = NUMBER;
    r->left->left->value = 1;
    r->left->left->sign = 1;
    r->left->left->parent = r->left;
    r->left->left->left = r->left->left->right = nullptr;

    /* (v^2+1) */
    r->left->right = new NMAST<T>;
    r->left->right->sign = 1;
    r->left->right->type = PLUS;
    r->left->right->parent = r->left;

    r->left->right->left = new NMAST<T>;
    r->left->right->left->type = POWER;
    r->left->right->left->value = 0;
    r->left->right->left->sign = 1;
    r->left->right->left->parent = r->left->right;
    r->left->right->left->left = cloneTree(v, r->left->right->left);

    r->left->right->left->right = new NMAST<T>;
    r->left->right->left->right->type = NUMBER;
    r->left->right->left->right->value = 2;
    r->left->right->left->right->sign = 1;
    r->left->right->left->right->parent = r->left->right->left;
    r->left->right->left->right->left = r->left->right->left->right->right = nullptr;

    r->left->right->right = new NMAST<T>;
    r->left->right->right->type = NUMBER;
    r->left->right->right->value = 1;
    r->left->right->right->sign = 1;
    r->left->right->right->parent = r->left->right;
    r->left->right->right->left = r->left->right->right->right = nullptr;

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;
    return r;
}

/*
* (u/v)' = (u'v - uv')/v^2
* */
template <typename T>
NMAST<T>* d_quotient(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;

    r = new NMAST<T>;
    r->text = "/";
    r->type = DIVIDE;
    r->value = 0;
    r->sign = 1;
    r->parent = nullptr;

    r->left = new NMAST<T>;
    r->left->text = "-";
    (r->left)->parent = r;
    (r->left)->type = MINUS;
    (r->left)->value = 0;
    (r->left)->sign = 1;

    /* ========================================== */
    (r->left)->left = new NMAST<T>;
    if (du != nullptr) {
        r->left->left->text = "*";
        (r->left)->left->type = MULTIPLY;
        (r->left)->left->sign = 1;
        (r->left)->left->parent = r->left;
        ((r->left)->left)->left = du;
        du->parent = (r->left)->left;
        ((r->left)->left)->right = cloneTree<T>(v, (r->left)->left);
    } else {
        r->left->left->text = "0";
        (r->left)->left->type = NUMBER;
        (r->left)->left->sign = 1;
        (r->left)->left->value = 0;
        (r->left)->left->parent = r->left;
        ((r->left)->left)->left = nullptr;
        ((r->left)->left)->right = nullptr;
    }
    /* =================================================== */

    (r->left)->right = new NMAST<T>;
    (r->left)->right->type = MULTIPLY;
    (r->left)->right->sign = 1;
    (r->left)->right->parent = r->left;
    ((r->left)->right)->left = cloneTree<T>(u, (r->left)->right);
    ((r->left)->right)->right = dv;
    if (dv != nullptr)
        dv->parent = (r->left)->right;

    /* ==================================================== */

    r->right = new NMAST<T>;
    (r->right)->parent = r;
    (r->right)->type = POWER;
    r->value = 0;
    r->sign = 1;

    (r->right)->left = cloneTree(v, r->right);

    (r->right)->right = new NMAST<T>;
    (r->right)->right->type = NUMBER;
    (r->right)->right->value = 2;
    (r->right)->right->sign = 1;
    (r->right)->right->parent = r->right;
    ((r->right)->right)->left = ((r->right)->right)->right = nullptr;
    return r;
} //2.0 got here

/*
* (u +- v) = u' +- v'
* */
template <typename T>
NMAST<T>* d_sum_subtract(NMAST<T> *t, int type, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;

    if(dv == 0 && du != 0) {
        return du;
    }

    if(dv != 0 && du == 0)
        return dv;

    r = new NMAST<T>;
    r->sign = 1;
    r->type = type;
    r->value = 0.0;
    r->parent = nullptr;

    r->left = du;
    if (du != nullptr)
        du->parent = r;

    r->right = dv;
    if (dv != nullptr)
        dv->parent = r;

    return r;
}

template <typename T>
NMAST<T>* d_pow_exp(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;
    int isXLeft = isContainVar(u, x);
    int isXRight = isContainVar(v, x);

    /* power: (u^a)' = au^(a-1)*u' */
    if (isXLeft != 0 && isXRight == 0){
        r = new NMAST<T>;
        r->sign = 1;
        r->type = MULTIPLY;
        r->value = 0.0;
        r->parent = nullptr;

        /* ===================================================== */
        r->left = new NMAST<T>;
        r->left->sign = 1;
        (r->left)->parent = r;
        (r->left)->type = MULTIPLY;

        (r->left)->left = new NMAST<T>;
        ((r->left)->left)->type = NUMBER;
        ((r->left)->left)->value = v->value;
        ((r->left)->left)->sign = v->sign;
        ((r->left)->left)->parent = (r->left);
        ((r->left)->left)->left = ((r->left)->left)->right = nullptr;

        /*printf(" Add Left Value 1/(v-1) \n");*/

        (r->left)->right = new NMAST<T>;
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
        if (du != nullptr) /* <-- is in need? */
            du->parent = r;
        /*printf(" Add Right du \n");*/

        return r;
    }

    /* power: (a^v)' = ln(a)*a^v*v' */
    if (isXLeft == 0 && isXRight != 0){
        r = new NMAST<T>;
        r->sign = 1;
        r->type = MULTIPLY;
        r->value = 0.0;
        r->parent = nullptr;

        /* ===================================================== */
        r->left = new NMAST<T>;
        (r->left)->parent = r;
        (r->left)->type = MULTIPLY;

        (r->left)->left = new NMAST<T>;
        ((r->left)->left)->type = LN;
        ((r->left)->left)->value = 0;
        ((r->left)->left)->sign = 1;
        ((r->left)->left)->parent = r->left;
        ((r->left)->left)->left = nullptr;
        ((r->left)->left)->right = cloneTree(u, (r->left)->left);

        (r->left)->right = new NMAST<T>;
        ((r->left)->right)->type = POWER;
        ((r->left)->right)->value = 0;
        ((r->left)->right)->sign = 1;
        ((r->left)->right)->parent = r->left;
        ((r->left)->right)->left = cloneTree(u, (r->left)->right);
        ((r->left)->right)->right = cloneTree(v, (r->left)->right);

        /* ===================================================== */
        r->right = dv;
        if (dv != nullptr)
            dv->parent = r;

        return r;
    }

    /* power: (u^v)' = (dv*ln(u) + v(du/u))*u^v */
    if (isXLeft != 0 && isXRight != 0){
        r = new NMAST<T>;
        r->sign = 1;
        r->type = MULTIPLY;
        r->value = 0.0;
        r->parent = nullptr;

        /* ===================================================== */
        r->left = new NMAST<T>;
        (r->left)->parent = r;
        (r->left)->type = PLUS;

        (r->left)->left = new NMAST<T>;
        ((r->left)->left)->type = MULTIPLY;
        ((r->left)->left)->value = 0;
        ((r->left)->left)->sign = 1;
        ((r->left)->left)->parent = r->left;

        ((r->left)->left)->left = dv;
        if (dv != nullptr)
            dv->parent = (r->left)->left;

        ((r->left)->left)->right = new NMAST<T>;
        ((r->left)->left)->right->sign = 1;
        ((r->left)->left)->right->type = LN;
        ((r->left)->left)->right->left = nullptr;
        ((r->left)->left)->right->right = cloneTree(u, ((r->left)->left)->right);

        (r->left)->right = new NMAST<T>;
        ((r->left)->right)->type = MULTIPLY;
        ((r->left)->right)->value = 0;
        ((r->left)->right)->sign = 1;
        ((r->left)->right)->parent = r->left;
        (r->left)->right->left = cloneTree(v, (r->left)->right);
        (r->left)->right->right = new NMAST<T>;
        (r->left)->right->right->sign = 1;
        (r->left)->right->right->type = DIVIDE;
        (r->left)->right->right->value = 0;
        (r->left)->right->right->left = du;
        if (du != nullptr)
            du->parent = (r->left)->right->right;
        (r->left)->right->right->right = cloneTree(u, (r->left)->right->right);

        /* ===================================================== */
        r->right = new NMAST<T>;
        r->right->sign = 1;
        r->right->type = POWER;
        r->right->left = cloneTree(u, r->right);
        r->right->right = cloneTree(v, r->right);

        return r;
    }

    return nullptr;
}


/* (sqrt(v))' = dv/(2*sqrt(v)) */
template <typename T>
NMAST<T>* d_sqrt(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    NMAST<T> *r;

    r = new NMAST<T>;
    r->type = DIVIDE;
    r->sign = 1;
    r->parent = nullptr;

    r->left = dv;
    if (dv != nullptr)
        dv->parent = r;

    /*Right child: product operator */
    r->right = new NMAST<T>;
    r->right->value = 0;
    r->right->sign = 1;
    r->right->type = MULTIPLY;
    r->right->parent = r;

    r->right->left = new NMAST<T>;
    r->right->left->type = NUMBER;
    r->right->left->value = 2;
    r->right->left->sign = 1;
    r->right->left->parent = r->right;
    r->right->left->right = r->right->left->left = nullptr;

    r->right->right = cloneTree(t, r->right);
    return r;
}

template <typename T>
NMAST<T>* d_ln(NMAST<T> *t, NMAST<T> *u, NMAST<T> *du, NMAST<T> *v, NMAST<T> *dv, string x){
    return nullptr;
}


template <typename T> int reduce_t(DParam<T> *dp) {
    NMAST<T> *p;

    /* If the tree is nullptr */
    if (dp == nullptr || dp->t == nullptr) {
        return 0;
    }

    DParam<T> this_param_left;
    DParam<T> this_param_right;
    if ( (dp->t->left != nullptr) && isFunctionOROperator(dp->t->left->type) ) {
        this_param_left.t = dp->t->left;
        reduce_t(&this_param_left);
    }
    if ((dp->t->right) != nullptr && isFunctionOROperator((dp->t->right)->type)){
        this_param_right.t = dp->t->right;
        reduce_t(&this_param_right);
    }
    if (this_param_left.error != 0) {
        dp->error = this_param_left.error;
        return dp->error;
    }
    if (this_param_right.error != 0) {
        dp->error = this_param_right.error;
        return dp->error;
    }
    /*
        We don't reduce a node if it's a variable, a NAME, a number, PI, E
    */
    if (((dp->t)->type == VARIABLE) || ((dp->t)->type == NAME) || isConstant((dp->t)->type)) {
        return dp->error;
    }

    /*
        So far, I have not cared about special cases for functions like SIN(PI), LN(E)
    */
    switch ((dp->t)->type) {

        case PLUS:
        case MINUS:
            dp->error = reduce_plus(dp->t);
            break;

        case MULTIPLY:
            dp->error = reduce_multiply(dp->t);
            break;

        case DIVIDE:
            dp->error = reduce_divide(dp->t);
            break;

        case POWER:
            dp->error = reduce_power(dp->t);
            break;

        case SIN:
        case COS:
        case TAN:
        case COTAN:
        case ASIN:
        case ACOS:
        case ATAN:
            dp->error = calculate_function(dp->t);
            break;

        case LN:
            if(dp->t->right->type == E_TYPE && dp->t->right->sign == 1 ) {

                dp->t->type = NUMBER;
                dp->t->text = "1";
                dp->t->value = (T)1;
                dp->t->priority = 0;

                p = dp->t->right;
                dp->t->right = nullptr;
                delete p;
                return dp->error;
            }
            dp->error = calculate_function(dp->t);
            break;

        case SQRT:
            dp->error = calculate_function(dp->t);
            break;

        case LOG:
            if (((dp->t)->left != nullptr) && ((dp->t)->right != nullptr) && isConstant(((dp->t)->left)->type)
                && isConstant(((dp->t)->right)->type)) {

                (dp->t)->value = doCalculate(((dp->t)->left)->value, ((dp->t)->right)->value, (dp->t)->type, &(dp->error));
                if (dp->error != 0) {
                    return dp->error;
                }
                (dp->t)->type = NUMBER;
                p = (dp->t)->left;
                delete p;
                p = (dp->t)->right;
                delete p;
                (dp->t)->left = (dp->t)->right = nullptr;
                return dp->error;
            }
            break;

        default:
            break;
    }
    return dp->error;
}

template <typename T> int calc_t(DParam<T> *dp) {
    NMAST<T> *t = dp->t;
    DParam<T> this_param_left;
    DParam<T> this_param_right;
    int var_index;

    this_param_left.error = this_param_right.error = 0;
    this_param_left.varCount = this_param_right.varCount = dp->varCount;
    for(unsigned int i=0; i < dp->varCount; i++) {
        this_param_left.variables[i] = this_param_right.variables[i] = dp->variables[i];
    }
    memcpy(this_param_left.values, dp->values, MAX_VAR_COUNT);
    memcpy(this_param_right.values, dp->values, MAX_VAR_COUNT);

    /* If the input tree is nullptr, we do nothing */
    if (t == nullptr) return 0;

    if (t->type == VARIABLE){
        var_index = isListContain(dp->variables, dp->varCount, t->text.c_str());
        dp->retv = (t->sign>0) ? (dp->values[var_index]) : (-dp->values[var_index]);
        return dp->error;
    }

    if ((t->type == NUMBER) || (t->type == PI_TYPE) || (t->type == E_TYPE)){
        dp->retv = t->value;
        return dp->error;
    }

    this_param_left.t = t->left;
    this_param_right.t = t->right;

    calc_t(&this_param_left);
    calc_t(&this_param_right);
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
    return dp->error;
}


template <typename T> int derivative(DParam<T> *dp) {
    NMAST<T> *t = dp->t;
    string x = dp->variables[0];
    NMAST<T> *u, *du, *v, *dv;
    DParam<T> pdu, pdv;

    dp->returnValue = nullptr;
    if (t == nullptr) {
        return 0;
    }

    if (t->type == NUMBER || t->type == PI_TYPE || t->type == E_TYPE || (!isContainVar(t, x)) ) {
        u = new NMAST<T>;
        u->sign = 1;
        u->type = NUMBER;
        u->value = 0.0;
        u->parent = nullptr;
        u->left = u->right = nullptr;
        u->text = "0";
        dp->returnValue = u;
        return 0;
    }

    /*
    IMPORTANT:
    In case of multi-variable function, we need to tell which variable that we are
    getting derivative of
    */
    if (t->type == VARIABLE) {
        u = new NMAST<T>;
        u->type = NUMBER;
        u->sign = 1;
        u->value = 1.0;
        u->parent = nullptr;
        u->left = u->right = nullptr;
        u->text = "";
        if (dp->variables[0] == t->text){
            u->value = 1.0;
            dp->returnValue = u;
            return 0;
        }
        u->value = 0.0;
        return 0;
    }

    dv = du = nullptr;

    u = t->left;
    v = t->right;

    bool u_contains_var = isContainVar(u, x);
    bool v_contains_var = isContainVar(v, x);

    if (u != nullptr && u_contains_var) {
        pdu.t = t->left;
        pdu.variables[0] = x;
        derivative(&pdu);
    }

    if (v != nullptr && v_contains_var) {
        pdv.t = t->right;
        pdv.variables[0] = x;
        derivative(&pdv);
    }

    /****************************************************************/
    // 2.0 get done here
    switch (t->type){
        case SIN:
            dp->returnValue = d_sin(t, u, du, v, dv, x);
            return 0;

        case COS:
            dp->returnValue = d_cos(t, u, du, v, dv, x);
            return 0;

        case TAN:
            dp->returnValue = d_tan(t, u, du, v, dv, dp->variables[0]);
            return 0;

        case COTAN:
            dp->returnValue = d_cotan(t, u, du, v, dv, dp->variables[0]);
            return 0;

        case ASIN:
            dp->returnValue = d_asin(t, u, du, v, dv, dp->variables[0]);
            return 0;

        case ACOS:
            dp->returnValue = d_acos(t, u, du, v, dv, dp->variables[0]);
            return 0;

        case ATAN:
            dp->returnValue = d_atan(t, u, du, v, dv, dp->variables[0]);
            return 0;

        case SQRT:
            dp->returnValue = d_sqrt(t, u, du, v, dv, x);
            return 0;

        case LN:
            dp->returnValue = d_ln(t, u, du, v, dv, x);
            return 0;

        case PLUS:
        case MINUS:
            dp->returnValue = d_sum_subtract<T>(t, t->type, u, du, v, dv, x);
            return 0;

        case MULTIPLY:
            if(!u_contains_var && v_contains_var) {
                dp->returnValue = new NMAST<T>;
                dp->returnValue->sign = 1;
                dp->returnValue->text = "*";
                dp->returnValue->parent = nullptr;
                dp->returnValue->value = 0;
                dp->returnValue->type = MULTIPLY;
                dp->returnValue->priority = getPriorityOfType(MULTIPLY);
                dp->returnValue->left = cloneTree(u, dp->returnValue);
                dp->returnValue->right = dv;
                if(du != 0) clearTree(&du);
            } else if(u_contains_var && !v_contains_var) {
                dp->returnValue = new NMAST<T>;
                dp->returnValue->sign = 1;
                dp->returnValue->text = "*";
                dp->returnValue->parent = nullptr;
                dp->returnValue->value = 0;
                dp->returnValue->type = MULTIPLY;
                dp->returnValue->priority = getPriorityOfType(MULTIPLY);
                dp->returnValue->left = cloneTree<T>(v, dp->returnValue);
                dp->returnValue->right = du;
                if(dv != 0) clearTree<T>(&dv);
            } else
                dp->returnValue = d_product<T>(t, u, du, v, dv, x);

            return 0;

        case DIVIDE:
            dp->returnValue = d_quotient<T>(t, u, du, v, dv, x);
            return 0;

        case POWER:
            dp->returnValue = d_pow_exp<T>(t, u, du, v, dv, x);
            return 0;
    }
    /* TODO: WHERE du AND dv GO IF WE DON'T TO USE THEM ????? */
    return 0;
}
}


#endif
