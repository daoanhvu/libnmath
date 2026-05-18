#ifndef _INTERNAL_COMMON_H
#define _INTERNAL_COMMON_H

#include "common_data.h"

namespace nmath {
    char getPriorityOfType(int type);
    bool isAFunctionType(int type);
    bool isAnOperatorType(int type);
    bool isFunctionOROperator(int type);
    bool isComparationOperator(int type);
    bool isConstant(int type);
    bool isDigit(char c);
    void getOperatorChar(int operatorType, char *opCh);
    bool isASCIILetter(char c);
    bool checkParenthesePrackets(char c, int *type);
    int isListContain(std::vector<std::string> list, const char *str);
    int isListContain(const std::string *list, int size, const char *str);
} //end namespace nmath

#endif
