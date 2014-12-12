#ifndef _NLABLEXER_H
#define _NLABLEXER_H

#include "common.h"

namespace nmath{
	class NLabLexer {
		private:
			static const int setLeadNegativeNumber[14];
			static const int LeadNegativeNumberSize = 14;

			char *mInputString;
			int mInputLen;			//input length in byte

			int errorCode;
			int errorColumn;

			unsigned int mLoggedSize;
			unsigned int mSize;
			Token *list;

			int isLogicOperator(char c);

			int addToken(int _type, const char *_text, char txtlen, int _col);
			int isNumericOperatorOREQ(char c);
			int parserLogicOperator(int idx, int *type, int *outlen);
			int checkNumericOperator(int idx, int *type, int *textLength);
			int parseSubtractSign(int idx, int *type, int *outlen);
			int isFunctionName(int index, int *outType, int *outlen);
			int isAName(int index);

		public:
			NLabLexer(int loggerSize);
			~NLabLexer();

			void reset(int logSize);
			int getErrorColumn()	{ return errorColumn; }

			int lexicalAnalysis(char *inStr, int len, int appended);
			int size() { return mSize; }
			Token* operator [](int index);
	};

	/**
	Check Parenthese or prackets

	Return TRUE if it's a Parenthese or prackets
	otherwise return FALSE

	In case it returns TRUE, type will hold the actual type of the token
	*/
	int checkParenthesePrackets(char c, int *type);

	/**
	Check comma or semicolon.
	Return TRUE if it's a semiconlon or comma
	otherwise return FALSE
	In case it returns TRUE, type will hold the actual type of the token
	*/
	int checkCommaSemi(char c, int *type);
}

#endif