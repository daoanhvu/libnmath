
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "internal_common.h"

char nmath::getPriorityOfType(int type) {
    switch(type){
        case OR:
            return (char)1;

        case AND:
            return (char)2;

        case LT:
        case GT:
        case LTE:
        case GTE:
            return (char)3;

        case PLUS:
        case MINUS:
            return (char)4;

        case MULTIPLY:
        case DIVIDE:
            return (char)5;

        case POWER:
            return (char)6;

        case NE:
            return (char)7;

        default:
            return (char)0;
    }
}

bool nmath::isAFunctionType(int type){
    int i;
    for(i=0; i<FUNCTION_COUNT; i++)
        if(type == FUNCTIONS[i])
            return true;
    return false;
}

bool nmath::isAnOperatorType(int type){
    int i;
    for(i=0; i<OPERATOR_COUNT; i++)
        if(type == OPERATORS[i])
            return true;
    return false;
}

bool nmath::isFunctionOROperator(int type){
    int i;
    for(i=0; i<FUNCTION_COUNT; i++)
        if(type == FUNCTIONS[i])
            return true;

    for(i=0; i<OPERATOR_COUNT; i++)
        if(type == OPERATORS[i])
            return true;

    return false;
}

bool nmath::isComparationOperator(int type){
    int i;
    for(i=0; i<COMPARING_OPERATORS_COUNT; i++)
        if(type == COMPARING_OPERATORS[i])
            return true;
    return false;
}

bool nmath::isConstant(int type) {
    return type == NUMBER || type == PI_TYPE || type == E_TYPE;
}

bool nmath::isDigit(char c) {
    return c >= '0' && c <= '9';
}

void nmath::getOperatorChar(int operatorType, char *opCh) {
    switch (operatorType){
        case PLUS:
            (*opCh) = '+';
            return;
        case MINUS:
            (*opCh) = '-';
            return;
        case MULTIPLY:
            (*opCh) = '*';
            return;
        case DIVIDE:
            (*opCh) = '/';
            return;
        case POWER:
            (*opCh) = '^';
            return;
        default:
            (*opCh) = 0;
            return;
    }
}

bool nmath::isASCIILetter(char c){
    return (c >= 'a' && c <= 'z' ) || (c >= 'A' && c <= 'Z' );
}

/**
	Check Parenthese or prackets

	Return TRUE if it's a Parenthese or prackets
	otherwise return FALSE

	In case it returns TRUE, type will hold the actual type of the token
*/
bool nmath::checkParenthesePrackets(char c, int *type) {

    switch(c) {

        case '(':
            *type = LPAREN;
            return true;

        case ')':
            *type = RPAREN;
            return true;

        case '[':
            *type = LPRACKET;
            return true;

        case ']':
            *type = RPRACKET;
            return true;

        default:
            return false;
    }
}

int nmath::isListContain(std::vector<std::string> list, const char *str) {
    int idx = -1;
    for(int i=0; i<list.size(); i++) {
        if(list[i] == str) {
            idx = i;
            break;
        }
    }
    return idx;
}

int nmath::isListContain(const std::string *list, int size, const char *str) {
    int idx = -1;
    for(auto i=0; i<size; i++) {
        if(list[i] == str) {
            idx = i;
            break;
        }
    }
    return idx;
}