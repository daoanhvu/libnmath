#ifndef _NLABPARSER_H
#define _NLABPARSER_H

#ifdef _PCDEBUG
#include <iostream>
#endif

#include <vector>
#include <string>
#include <cstdlib>
#include "nlablexer.h"
#include "common.hpp"
#include "StackUtil.h"

namespace nmath {
	template <typename T>
	class NLabParser {
	private:
        NMASTPool<T> *nmastPool;
        bool needReleasePool;
	public:
        NLabParser() {
            nmastPool = new NMASTPool<T>;
            needReleasePool = true;
        }

        NLabParser(NMASTPool<T> *aPool): nmastPool(aPool), needReleasePool(false) {
        }

        virtual ~NLabParser() {
            if(needReleasePool) {
                delete nmastPool;
            }
        }

        int getType(const Token *t, std::vector<nmath::NMAST<T>* > variables);
        int functionNotation(std::vector<Token*> tokens, int index,
                             std::vector<nmath::NMAST<T> *> &variables,
                             int *errorCode, int *errorColumn);
        NMAST<T>* buildIntervalTree(Token* valtk1, Token* o1, Token* variable,
                                    Token* o2, Token* valtk2, int *errorCode, int *errorColumn);

        int parseFunctionExpression(std::vector<Token*> tokens, std::vector<nmath::NMAST<T>* > &prefix, std::vector<nmath::NMAST<T>* > &domain,
                                    std::vector<nmath::NMAST<T>* > &variables, int *errorCode, int *errorColumn);
        NMAST<T>* parseExpression(std::vector<Token*> tokens, int *start, std::vector<nmath::NMAST<T>*> variables, int *errorCode, int *errorColumn);
        NMAST<T>* parseDomain(std::vector<Token*> tokens, int *start, int *errorCode, int *errorColumn);
	};


    template <typename T>
    int NLabParser<T>::getType(const Token *t, std::vector<nmath::NMAST<T>* > variables) {
        for(int i=0; i<variables.size(); i++) {
            if( strcmp(t->text, variables[i]->text) == 0 ) {
                return VARIABLE;
            }
        }
        return t->type;
    }

    /******************************************************************************************/

    /**
     *  Parse the an input token list to function expressed with NMAST
     *  Return the errorCode 
     */
    template <typename T>
    int NLabParser<T>::parseFunctionExpression(std::vector<Token*> tokens,
            std::vector<nmath::NMAST<T>* > &prefix,
            std::vector<nmath::NMAST<T>* > &domain,
            std::vector<nmath::NMAST<T>* > &variables,
            int *errorCode, int *errorColumn) {
        int k, l, idx = 0;
        *errorCode = ERROR_NOT_A_FUNCTION;
        if(tokens.size() <= 0) {
            *errorColumn = 0;
            return *errorCode;
        }
        *errorColumn = tokens[idx]->column;
        auto tokenCount = tokens.size();
        nmath::NMAST<T> *item;

        /** This array will hold the variables of the function */
        if ((k = functionNotation(tokens, idx, variables, errorCode, errorColumn)) > idx) {
            if(tokens[k]->type == EQ) {
                k++;
                do {
                    /*
                        Parse expression
                    */
                    if (tokenCount <= k) {
                        *errorCode = ERROR_NOT_AN_EXPRESSION;
                        *errorColumn = k;
                        return ERROR_NOT_AN_EXPRESSION;
                    }

                    //
                    for(int j = k; j < tokenCount; j++) {
                        if(tokens[j]->type == NAME) {
                            for(int v = 0; v < variables.size(); v++) {
                                if(tokens[j]->text == variables[v]->text) {
                                    tokens[j]->type = VARIABLE;
                                }
                            }
                        }
                    }

                    item = parseExpression(tokens, &k, variables, errorCode, errorColumn);
                    /** after parseExpression, we may get error, so MUST check if it's OK here */
                    if( *errorCode != NMATH_NO_ERROR ) {
                        break;
                    }

                    prefix.push_back(item);

                    item = nullptr;
                    if( (k < tokenCount) && (tokens[k]->type == DOMAIN_NOTATION) ) {
                        *errorCode = ERROR_MISSING_DOMAIN;
                        *errorColumn = tokens[k]->column;
                        if(k+1 < tokenCount) {
                            l = k + 1;
                            item = parseDomain(tokens, &l, errorCode, errorColumn);
                            k = l;
                        }
                    }

                    domain.push_back(item);

                } while ( errorCode==NMATH_NO_ERROR && k < tokenCount );
            }
        }

        if(*errorCode != NMATH_NO_ERROR) {
            for(k=0; k<prefix.size(); k++) {
                clearTree(&(prefix[k]));

                if(domain[k] != nullptr){
                    clearTree(&(domain[k]));
                }
            }
            prefix.clear();
            domain.clear();
        }
        return *errorCode;
    }

    /**
        functionNotation: NAME LPAREN NAME (COMA NAME)* PRARENT;
        @Param index the starting position in token list where this function need to check this pattern
        @return
            - If the tokens match the pattern the function will return the next index, this index equals
            input index plus the length of the matched tokens.
            - If not, the function return the input index
    */
    template <typename T>
    int NLabParser<T>::functionNotation(std::vector<Token*> tokens, int index,
                                        std::vector<nmath::NMAST<T>* > &variables,
        int *errorCode, int *errorColumn) {
        int oldIndex = index;
        auto tokenCount = tokens.size();

        *errorCode = ERROR_NOT_A_FUNCTION;
        *errorColumn = tokens[index]->column;

        if( (tokenCount < 4) || (index < 0) || (index >= tokenCount-3) )
            return index;

        if(tokens[index]->type == NAME ) {
            *errorCode = ERROR_PARENTHESE_MISSING;
            if(tokens[index+1]->type == LPAREN) {
                *errorCode = ERROR_MISSING_VARIABLE;
                *errorColumn = tokens[index+1]->column;

                if(tokens[index+2]->type == NAME) {
                    tokens[index+2]->type = VARIABLE;
                    NMAST<T>* var = nmastPool->get();
                    var->type = VARIABLE;
                    var->text = tokens[index+2]->text;
                    variables.push_back(var);
                    index += 3;
                    while( (index+1<tokenCount) && (tokens[index]->type == COMMA)
                            && (tokens[index+1]->type == NAME ) ) {
                        tokens[index+1]->type = VARIABLE;
                        var = nmastPool->get();
                        var->type = VARIABLE;
                        var->text = tokens[index+1]->text;
                        variables.push_back(var);
                        index += 2;
                    }
                    *errorCode = ERROR_PARENTHESE_MISSING;
                    *errorColumn = tokens[index]->column;
                    if( (index < tokenCount) && (tokens[index]->type == RPAREN)){
                        *errorCode = NMATH_NO_ERROR;
                        *errorColumn = -1;
                        return (index + 1);
                    }
                }
            }
        }
        return oldIndex;
    }//done

    /******************************************************************************************/
    /**
        Parse the input string in object f to NMAST tree using PRN
    */
    template <typename T>
    nmath::NMAST<T>* NLabParser<T>::parseExpression(vector<Token*> tokens,
                                             int *start, std::vector<nmath::NMAST<T>* > variables, int *errorCode, int *errorColumn) {
        int idx, top=-1, allocLen=0;
        bool isEndExp = false;
        int error;
        T val;
        size_t size = tokens.size();
        Token *tk = nullptr;
        Token **stack = nullptr;
        Token *stItm = nullptr;
        NMAST<T> *ast = nullptr;
        std::vector<NMAST<T>* >mPrefix;

        *errorColumn = -1;
        *errorCode = NMATH_NO_ERROR;

        idx = (*start);
        while( (idx < size) && !isEndExp) {
            tk = tokens[idx];
            switch(tk->type) {
            case NUMBER:
                val = parseDouble<T>(tk->text, 0, tk->textLength, &error);
                if(val == (T)0 && error < 0) {
                    clearStackWithoutFreeItem(stack, top+1);
                    free(stack);
                    for (int i = 0; i<mPrefix.size(); i++)
                        clearTree(&(mPrefix[i]));
                    mPrefix.clear();
                    *errorColumn = tk->column;
                    *errorCode = ERROR_PARSING_NUMBER;
                    return nullptr;
                }

                ast = nmastPool->get();
                ast->value = val;
                ast->type = tk->type;
                ast->text = tk->text;
                mPrefix.push_back(ast);
                idx++;
            break;

            case E_TYPE:
                ast = nmastPool->get();
                ast->value = E;
                ast->text = "e";
                ast->type = E_TYPE;
                mPrefix.push_back(ast); //add this item to prefix
                idx++;
            break;

            case PI_TYPE:
                ast = nmastPool->get();
                ast->value = PI;
                ast->text = "3.14159";
                ast->type = PI_TYPE;
                mPrefix.push_back(ast); //add this item to prefix
                idx++;
            break;


            case PLUS:
            case MINUS:
            case MULTIPLY:
            case DIVIDE:
            case POWER:
                /**
                 * When we meet an arithmetic operator, let call it o1, 
                 * C1: we check if there is an arithmetic operator o2
                 * at the top of the stack and o2.priotiry >= o1.priority (*)
                 * then we take o2 out of the stack, and take operand1 and operand2 from postfix
                 * and do the calculate operand1 operator2 operand2 and then we push the result
                 * back to the postfix
                 * - Repeat (C1) until (*) not hold
                 */ 
                if(top >= 0) {
                    stItm = stack[top];
                    while(isAnOperatorType(stItm->type) && (stItm->priority) >= tk->priority) {
                        stItm = popFromStack(stack, &top);
                        if (mPrefix.size() == 1) {
                            mPrefix[0]->sign = -1;
                        } else {
                            ast = nmastPool->get();
                            ast->type = stItm->type;
                            ast->text = stItm->text;
                            ast->priority = stItm->priority;
                            ast->left = mPrefix[mPrefix.size() - 2];
                            ast->right = mPrefix[mPrefix.size() - 1];

                            if((ast->left)!=nullptr)
                                (ast->left)->parent = ast;
                            if((ast->right)!=nullptr)
                                (ast->right)->parent = ast;

                            mPrefix[mPrefix.size() - 2] = ast;
                            mPrefix[mPrefix.size() - 1] = nullptr;
                            mPrefix.pop_back();
                        }

                        if(top < 0)
                            break;

                        stItm = stack[top];
                    }
                }
                //push operation o1 (tk) into stack
                pushItem2Stack(&stack, &top, &allocLen, tk);
                if(*errorCode == E_NOT_ENOUGH_MEMORY) {
                    clearStackWithoutFreeItem(stack, top+1);
                    free(stack);
                    for (int i = 0; i<mPrefix.size(); i++)
                        clearTree(&(mPrefix[i]));
                    mPrefix.clear();
                    *errorColumn = tk->column;
                    return nullptr;
                }
                idx++;
            break;

            case LPAREN:/*If it an open parentheses then put it to stack*/
                pushItem2Stack(&stack, &top, &allocLen, tk);
                if(*errorCode == E_NOT_ENOUGH_MEMORY) {
                    clearStackWithoutFreeItem(stack, top+1);
                    free(stack);
                    for (int i = 0; i<mPrefix.size(); i++)
                        clearTree(&(mPrefix[i]));
                    mPrefix.clear();
                    *errorColumn = tk->column;
                    return nullptr;
                }
                idx++;
            break;

            case RPAREN:
                stItm = popFromStack(stack, &top);

                /* got an opening-parenthese but can not find a closing-parenthese */
                if(stItm == nullptr) {
                    clearStackWithoutFreeItem(stack, top+1);
                    free(stack);
                    for (int i = 0; i<mPrefix.size(); i++)
                        clearTree(&(mPrefix[i]));
                    mPrefix.clear();
                    *errorColumn = tk->column;
                    *errorCode = ERROR_PARENTHESE_MISSING;
                    return nullptr;
                }

                /*  */
                while( (stItm != nullptr) && (stItm->type != LPAREN) && !isAFunctionType(stItm->type) ) {
                    addFunction2Tree<T>(mPrefix, stItm, nmastPool);
                    //free(stItm);
                    stItm = popFromStack(stack, &top);
                }

                /* got an opening-parenthese but can not find a closing-parenthese */
                if(stItm == nullptr) {
                    clearStackWithoutFreeItem(stack, top+1);
                    free(stack);
                    for (int i = 0; i<mPrefix.size(); i++)
                    clearTree(&(mPrefix[i]));
                    mPrefix.clear();
                    *errorColumn = tk->column;
                    *errorCode = ERROR_PARENTHESE_MISSING;
                    return nullptr;
                }

                if(isAFunctionType(stItm->type)) {
                    addFunction2Tree<T>(mPrefix, stItm, nmastPool);
                }
                idx++;
            break;

            //functions
            case SIN:
            case COS:
            case TAN:
            case COTAN:
            case ASIN:
            case ACOS:
            case ATAN:
            case SQRT:
            case LN:
            case LOG:
            case ABS:
                pushItem2Stack(&stack, &top, &allocLen, tk);
                if(*errorCode == E_NOT_ENOUGH_MEMORY) {
                    clearStackWithoutFreeItem(stack, top+1);
                    free(stack);
                    for (int i = 0; i<mPrefix.size(); i++)
                        clearTree(&(mPrefix[i]));
                    mPrefix.clear();
                    *errorColumn = tk->column;
                    return nullptr;
                }
                /*
                    After a function name must be a LPAREN, and we just ignore that LPAREN token
                    idx += 2;
                */
                // the left-parenthese right after a function name is dimissed, so just don't care of it
                idx++;
            break;

            case NAME:
            case VARIABLE:
                ast = nmastPool->get();
                ast->text = tk->text;
                ast->type = tk->type;
                mPrefix.push_back(ast);
                idx++;
            break;

            case SEMI:
            case DOMAIN_NOTATION:
                /* 	End of this expression, kindly stop the while loop, consume
                    this expression and start processing the next expression.
                */
                isEndExp = true;
            break;

            default:
                clearStackWithoutFreeItem(stack, top+1);
                free(stack);
                for (int i = 0; i<mPrefix.size(); i++)
                    clearTree(&(mPrefix[i]));
                mPrefix.clear();
                *errorColumn = tk->column;
                *errorCode = ERROR_BAD_TOKEN;
                return nullptr;
            }//end switch
        }//end while

        while( top >= 0 ) {
            stItm = popFromStack(stack, &top);
            if( (stItm->type == LPAREN) || isAFunctionType(stItm->type)) {
                clearStackWithoutFreeItem(stack, top+1);
                free(stack);
                for (int i = 0; i<mPrefix.size(); i++)
                    clearTree(&(mPrefix[i]));
                mPrefix.clear();
                *errorColumn = (tk != nullptr) ? tk->column : -1;
                *errorCode = ERROR_PARENTHESE_MISSING;
                return nullptr;
            }
            addFunction2Tree(mPrefix, stItm, nmastPool);
        }

        free(stack);
        *start = idx;
        ast = mPrefix[0];
        mPrefix.clear();
        return ast;
    }

    template <typename T>
    NMAST<T>* NLabParser<T>::parseDomain(std::vector<Token*> tokens, int *start, int *errorCode, int *errorColumn) {
        auto isEndExp = false;
        int i, index, top = -1, allocLen=0;
        Token* tk = nullptr;
        double val = 0, val2 = 0;
        Token **stack = nullptr;
        Token *tokenItm = nullptr;
        NMAST<T> *ast;
        auto tokenCount = tokens.size();

        std::vector<NMAST<T>* > mDomain;

        *errorColumn = -1;
        *errorCode = NMATH_NO_ERROR;

        index = *start;
        while( (index < tokenCount) && !isEndExp) {
            tk = tokens[index];
            switch(tk->type) {
                case NUMBER:
                case PI_TYPE:
                case E_TYPE:
                    if( (index+4)<tokenCount && isComparationOperator(tokens[index+1]->type)
                        && tokens[index+2]->type==VARIABLE
                        && isComparationOperator(tokens[index+3]->type)
                        && (tokens[index+4]->type==NUMBER || tokens[index+4]->type==PI_TYPE || tokens[index+4]->type==E_TYPE )) {
                        /*
                            HERE, I missed the case that NUMBER < VARIABLE < NUMBER or
                            NUMBER <= VARIABLE < NUMBER or NUMBER < VARIABLE <= NUMBER or
                            NUMBER <= VARIABLE <= NUMBER

                            Build an AND tree to hold the case
                        */
                        ast = buildIntervalTree(tk, tokens[index+1],
                                                tokens[index+2], tokens[index+3], tokens[index+4], errorCode, errorColumn);
                        if (ast == nullptr) {
                            clearStackWithoutFreeItem(stack, top+1);
                            free(stack);
                            for (i = 0; i<mDomain.size(); i++)
                                clearTree(&(mDomain[i]));
                            mDomain.clear();
                            return nullptr;
                        }
                        mDomain.push_back(ast);
                        index += 5;
                    }// end if

                    switch(tk->type){
                        case NUMBER:
                            val = parseDouble<T>(tk->text, 0, tk->textLength, errorCode);
                            if(val == 0 && *errorCode != NMATH_NO_ERROR){
                                clearStackWithoutFreeItem(stack, top+1);
                                free(stack);
                                for (i = 0; i<mDomain.size(); i++)
                                    clearTree(&(mDomain[i]));
                                mDomain.clear();
                                *errorColumn = tk->column;
                                return nullptr;
                            }
                            break;
                        case PI_TYPE:
                            val = PI;
                            break;
                        case E_TYPE:
                            val = E;
                            break;

                        default:
                            val = 0;
                    }

                    ast = nmastPool->get();
                    ast->value = val;
                    ast->type = tk->type;
                    mDomain.push_back(ast);
                    index++;
                    break;

                case LT:
                case LTE:
                case GT:
                case GTE:
                case AND:
                case OR:
                    if(top >= 0){
                        tokenItm = stack[top];
                        while((isAnOperatorType(tokenItm->type) || isComparationOperator(tokenItm->type) || tokenItm->type==AND || tokenItm->type==OR)
                              && (tokenItm->priority) >= tk->priority){
                            tokenItm = popFromStack(stack, &top);

                            ast = nmastPool->get();
                            ast->type = tokenItm->type;
                            ast->priority = tokenItm->priority;
                            ast->left = mDomain[mDomain.size() - 2];
                            ast->right = mDomain[mDomain.size() - 1];
                            if((ast->type == LT || ast->type == LTE || ast->type == GT || ast->type == GTE )
                               && (mDomain[mDomain.size() - 1]->type == VARIABLE)){
                                ast->left = mDomain[mDomain.size() - 1];
                                ast->right = mDomain[mDomain.size() - 2];

                                switch(ast->type){
                                    case LT:
                                        ast->type = GT;
                                        break;

                                    case LTE:
                                        ast->type = GTE;
                                        break;

                                    case GT:
                                        ast->type = LT;
                                        break;

                                    case GTE:
                                        ast->type = LTE;
                                        break;

                                    default:;
                                }


                            }

                            if((ast->left)!=nullptr)
                                (ast->left)->parent = ast;
                            if((ast->right)!=nullptr)
                                (ast->right)->parent = ast;

                            mDomain[mDomain.size() - 2] = ast;
                            mDomain[mDomain.size() - 1] = nullptr;
                            mDomain.pop_back();

                            if(top < 0)
                                break;

                            tokenItm = stack[top];
                        }
                    }
                    //push operation o1 (tk) into stack
                    pushItem2Stack(&stack, &top, &allocLen, tk);
                    if(*errorCode == E_NOT_ENOUGH_MEMORY){
                        clearStackWithoutFreeItem(stack, top+1);
                        free(stack);
                        for (i = 0; i<mDomain.size(); i++)
                            clearTree(&(mDomain[i]));
                        mDomain.clear();
                        *errorColumn = tk->column;
                        return nullptr;
                    }
                    index++;
                    break;

                case RPAREN:
                    tokenItm = popFromStack(stack, &top);

                    /* got an opening-parenthese but can not find a closing-parenthese */
                    if(tokenItm == nullptr) {
                        clearStackWithoutFreeItem(stack, top+1);
                        free(stack);
                        for (i = 0; i<mDomain.size(); i++)
                            clearTree(&(mDomain[i]));
                        mDomain.clear();
                        *errorColumn = tk->column;
                        *errorCode = ERROR_PARENTHESE_MISSING;
                        return nullptr;
                    }

                    /*  */
                    while(tokenItm!=nullptr && (tokenItm->type != LPAREN) && !isAFunctionType(tokenItm->type) ){
                        addFunction2Tree<T>(mDomain, tokenItm, nmastPool);
                        tokenItm = popFromStack(stack, &top);
                    }

                    if(tokenItm==nullptr){
                        /* ERROR: got an opening-parenthese but can not find a closing-parenthese */
                        clearStackWithoutFreeItem(stack, top+1);
                        free(stack);
                        for (i = 0; i<mDomain.size(); i++)
                            clearTree(&(mDomain[i]));
                        mDomain.clear();
                        *errorColumn = tk->column;
                        *errorCode = ERROR_PARENTHESE_MISSING;
                        return nullptr;
                    }

                    if(isAFunctionType(tokenItm->type)) {
                        addFunction2Tree<T>(mDomain, tokenItm, nmastPool);
                    }

                    index++;
                    break;

                case LPAREN:
                    pushItem2Stack(&stack, &top, &allocLen, tk);
                    if(*errorCode == E_NOT_ENOUGH_MEMORY){
                        clearStackWithoutFreeItem(stack, top+1);
                        free(stack);
                        for (i = 0; i<mDomain.size(); i++)
                            clearTree(&(mDomain[i]));
                        mDomain.clear();
                        *errorColumn = tokens[index]->column;
                        return nullptr;
                    }
                    index++;
                    break;

                case NAME:
                case VARIABLE:
                    if(( (index+1) < tokenCount) && tokens[index+1]->type == ELEMENT_OF){
                        /*
                            VARIABLE ELEMENT_OF [NUMBER,NUMBER]
                            VARIABLE ELEMENT_OF (NUMBER,NUMBER]
                            VARIABLE ELEMENT_OF [NUMBER,NUMBER)
                            VARIABLE ELEMENT_OF (NUMBER,NUMBER)
                        */
                        if( ( index+6 < tokenCount) && (tokens[index+2]->type == LPRACKET || tokens[index+2]->type == LPAREN)
                            && (tokens[index+3]->type == NUMBER || tokens[index+3]->type == PI_TYPE || tokens[index+3]->type == E_TYPE)
                            && tokens[index+4]->type == COMMA
                            && (tokens[index + 5]->type == NUMBER || tokens[index + 5]->type == PI_TYPE || tokens[index + 5]->type == E_TYPE)
                            && (tokens[index + 6]->type == RPRACKET || tokens[index + 6]->type == RPAREN)) {


                            /* ======== START Parse floating point values ======= */
                            switch(tokens[index+3]->type) {
                                case NUMBER:
                                    val = parseDouble<T>(tokens[index+3]->text, 0, tokens[index+3]->textLength, errorCode);
                                    if(val == 0 && *errorCode != NMATH_NO_ERROR){
                                        clearStackWithoutFreeItem(stack, top+1);
                                        free(stack);
                                        for(i=0;i<mDomain.size();i++)
                                            clearTree(&(mDomain[i]));
                                        mDomain.clear();
                                        *errorColumn = tk->column;
                                        return nullptr;
                                    }
                                    break;

                                case PI_TYPE:
                                    val = PI;
                                    break;
                                case E_TYPE:
                                    val = E;
                                    break;
                            }

                            switch(tokens[index+5]->type){
                                case NUMBER:
                                    val2 = parseDouble<T>(tokens[index + 5]->text, 0, tokens[index + 5]->textLength, errorCode);
                                    if(val2 == 0 && *errorCode != NMATH_NO_ERROR){
                                        clearStackWithoutFreeItem(stack, top+1);
                                        free(stack);
                                        for (i = 0; i<mDomain.size(); i++)
                                            clearTree(&(mDomain[i]));
                                        mDomain.clear();
                                        *errorColumn = tokens[index + 4]->column;
                                        return nullptr;
                                    }
                                    break;

                                case PI_TYPE:
                                    val2 = PI;
                                    break;
                                case E_TYPE:
                                    val2 = E;
                                    break;
                            }
                            /* ========END parsing floating point values=====*/

                            /*
                                This case, mX element_of [number1, number2]
                                We parse to a tree that have root got:
                                    - type GT_LT or GTE_LT or GT_TLE or GTE_LTE and
                                    - variable = mX
                            */
                            ast = nmastPool->get();
                            ast->text = tk->text;
                            if ((tokens[index + 2]->type == LPAREN) && (tokens[index + 6]->type == RPAREN))
                                ast->type = GT_LT;
                            else if ((tokens[index + 2]->type == LPRACKET) && (tokens[index + 6]->type == RPAREN))
                                ast->type = GTE_LT;
                            else if ((tokens[index + 2]->type == LPAREN) && (tokens[index + 6]->type == RPRACKET))
                                ast->type = GT_LTE;
                            else if ((tokens[index + 2]->type == LPRACKET) && (tokens[index + 6]->type == RPRACKET))
                                ast->type = GTE_LTE;

                            //ast->Left number 1
                            ast->left = nmastPool->get();
                            ast->left->parent = ast;
                            ast->left->value = val;
                            ast->left->type = tokens[index + 3]->type;
                            //Left->Right NUMBER or PI_TYPE or E_TYPE
                            ast->right = nmastPool->get();
                            ast->right->parent = ast;
                            ast->right->value = val2;
                            ast->right->type = tokens[index + 5]->type;
                            mDomain.push_back(ast);
                            index += 7;
                        } else {
                            /**
                                ERROR:After ELEMENT_OF is not a right syntax
                            */
                            clearStackWithoutFreeItem(stack, top+1);
                            free(stack);
                            for (i = 0; i<mDomain.size(); i++)
                                clearTree(&(mDomain[i]));
                            mDomain.clear();
                            *errorColumn = tk->column;
                            *errorCode = ERROR_SYNTAX;
                            return nullptr;
                        }
                    } else {
                        // VARIABLE OPERATOR VALUE
                        if(isComparationOperator(tokens[index+1]->type)) {
                            if( isConstant(tokens[index+2]->type) ) {
                                //OPERATOR
                                ast = nmastPool->get();
                                ast->type = tokens[index + 1]->type;
                                ast->text = tk->text; //Variable or NAME is stored at operator node
                                ast->value = parseDouble<T>(tokens[index+2]->text, 0, tokens[index+2]->textLength, errorCode);

                                mDomain.push_back(ast);
                                index += 3;
                            }
                        } else {
                            ast = nmastPool->get();
                            ast->text= tk->text;
                            ast->value = 0;
                            ast->type = tk->type;
                            mDomain.push_back(ast);
                            index++;
                        }
                    }
                    break;

                case SEMI:
                    isEndExp = true;
                    break;
            }
        }
        while(top >= 0) {
            tokenItm = popFromStack(stack, &top);

            if(tokenItm->type == LPAREN || isAFunctionType(tokenItm->type)){
                clearStackWithoutFreeItem(stack, top+1);
                free(stack);
                for (i = 0; i<mDomain.size(); i++)
                    clearTree(&(mDomain[i]));
                mDomain.clear();
                *errorColumn = (tk != nullptr) ? tk->column : -1;
                *errorCode = ERROR_PARENTHESE_MISSING;
                return nullptr;
            }
            addFunction2Tree<T>(mDomain, tokenItm, nmastPool);
        }
        *start = index;
        free(stack);
        ast = mDomain[0];
        mDomain.clear();
        return ast;
    }

    template <typename T>
    NMAST<T>* NLabParser<T>::buildIntervalTree(Token* valtk1, Token* o1, Token* variable,
                                               Token* o2, Token* valtk2, int *errorCode, int *errorColumn) {
        NMAST<T>* ast = nullptr;
        NMAST<T> *valNode1, *valNode2;
        T val1, val2;
        int type;
        bool isSwap = false;

        /** ERROR cases: -3 < x > 3 or -3 > x < 3  */
        if( ((o1->type == LT || o1->type == LTE) && (o2->type == GT || o2->type == GTE))
            || ((o1->type == GT || o1->type == GTE) && (o2->type == LT || o2->type == LTE)) ) {
            *errorCode = ERROR_SYNTAX;
            *errorColumn = o2->column;
            return nullptr;
        }

        /** ======================================================================== */
        switch(valtk1->type){
            case NUMBER:
                val1 = parseDouble<T>(valtk1->text, 0, valtk1->textLength, errorCode);
                if(val1 == 0 && *errorCode != NMATH_NO_ERROR){
                    *errorColumn = valtk1->column;
                    return nullptr;
                }
                break;

            case PI_TYPE:
                val1 = PI;
                break;
            case E_TYPE:
                val1 = E;
                break;
            default:
                val1 = (T)0;
        }

        switch(valtk2->type){
            case NUMBER:
                val2 = parseDouble<T>(valtk2->text, 0, valtk2->textLength, errorCode);
                if(val2 == 0 && *errorCode != NMATH_NO_ERROR){
                    *errorColumn = valtk2->column;
                    return nullptr;
                }
                break;

            case PI_TYPE:
                val2 = PI;
                break;
            case E_TYPE:
                val2 = E;
                break;

            default:
                val2 = (T)0;
        }

        valNode1 = nmastPool->get();
    //	valNode1->text[0] = '\0';
        valNode1->type = valtk1->type;
        valNode1->value = val1;
        valNode1->left = valNode1->right = nullptr;
        valNode2 = nmastPool->get();
    //	valNode2->text[0] = '\0';
        valNode2->type = valtk2->type;
        valNode2->value = val2;
        valNode2->left = valNode2->right = nullptr;

        /** ================================================================ */
        if((o1->type == LT ) && (o2->type == LT))
            type = GT_LT;
        else if((o1->type == LTE ) && (o2->type == LT))
            type = GTE_LT;
        else if((o1->type == LT ) && (o2->type == LTE))
            type = GT_LTE;
        else if((o1->type == LTE ) && (o2->type == LTE))
            type = GTE_LTE;
        else{
            isSwap = true;
            if((o1->type == GT ) && (o2->type == GT))
                type = GT_LT;
            else if((o1->type == GTE ) && (o2->type == GT))
                type = GT_LTE;
            else if((o1->type == GT ) && (o2->type == GTE))
                type = GTE_LT;
            else if((o1->type == GTE ) && (o2->type == GTE))
                type = GTE_LTE;
            else
                // TODO: check here please!!!
                type = TYPE_FLOATING_POINT;
        }

        //ast: GTE_LT | GT_TL
        ast = nmastPool->get();
        ast->type = type;
        ast->priority = 0;
        ast->text = variable->text;
        ast->parent = nullptr;
        if(isSwap) {
            ast->left = valNode2;
            valNode2->parent = ast;

            ast->right = valNode1;
            valNode1->parent = ast;
        } else {
            ast->left = valNode1;
            valNode1->parent = ast;

            ast->right = valNode2;
            valNode2->parent = ast;
        }
        return ast;
    }

}

#endif