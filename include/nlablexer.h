#ifndef _NLABLEXER_H
#define _NLABLEXER_H

#include "common_data.h"
#include <vector>
#include <string>

/*
	This class is an utility. It not generate any data so we don't need to release resource after
	use it.
*/
using namespace std;

namespace nmath{
    class NLabLexer {
    private:
		static const int setLeadNegativeNumber[14];
		static const int LeadNegativeNumberSize = 14;

		int errorCode;
		int errorColumn;

		bool isLogicOperator(char c);
		bool isDelimiterChar(char c);

		Token* createToken(int _type, const char *_text, int len, int _col);
		bool isNumericOperatorOREQ(char c);
		bool parseNumber(const char* inStr, int len, int idx, int *type, int *outlen);
		bool parserLogicOperator(const char *inStr, int idx, int *type, int *outlen);
		bool checkNumericOperator(const char *mInputString, int mInputLen, int idx, int *type, int *textLength, vector<Token*> list);
		bool parseSubtractSign(const char *mInputString, int mInputLen, int idx, int *type,
                                  vector<Token*> mList, int *outlen);
		bool isFunctionName(const char *mInputString, int mInputLen, int index, int *outType, int *outlen);
        bool isAName(const char* inStr, int inputLen, int idx, int *tlen);

    public:

		int getErrorCode() { return errorCode; }
		int getErrorColumn()	{ return errorColumn; }

		/*

		*/
		size_t lexicalAnalysis(const char *inStr, int len,
										   bool appended, int start, std::vector<Token*> &tokens, int *lastMeanIdx);

		size_t lexicalAnalysis(std::string inStr,
										   bool appended, int start, std::vector<Token*> &tokens, int *lastMeanIdx);
	};

	/**
        Check Parenthese or prackets
        Return TRUE if it's a Parenthese or prackets
        otherwise return FALSE
        In case it returns TRUE, type will hold the actual type of the token
	*/
	bool checkParenthesePrackets(char c, int *type);

	/**
        Check comma or semicolon.
        Return TRUE if it's a semiconlon or comma
        otherwise return FALSE
        In case it returns TRUE, type will hold the actual type of the token
	*/
	bool checkCommaSemi(char c, int *type);
}

#endif