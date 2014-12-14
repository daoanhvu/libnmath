#ifndef _NLABLEXER_H
#define _NLABLEXER_H

#include "common.h"

/*
	This class is an utility. It not generate any data so we don't need to release resource after
	use it.
*/
namespace nmath{
	class NLabLexer {
		private:
			static const int setLeadNegativeNumber[14];
			static const int LeadNegativeNumberSize = 14;

			const char *mInputString;
			int mInputLen;			//input length in byte

			int errorCode;
			int errorColumn;

			int mSize;
			int mCapability;
			Token *mList;

			int isLogicOperator(char c);

			int addToken(int _type, const char *_text, char txtlen, int _col);
			int isNumericOperatorOREQ(char c);
			int parserLogicOperator(int idx, int *type, int *outlen);
			int checkNumericOperator(int idx, int *type, int *textLength);
			int parseSubtractSign(int idx, int *type, int *outlen);
			int isFunctionName(int index, int *outType, int *outlen);
			int isAName(int index);

		public:
			NLabLexer();
			~NLabLexer();

			void reset();
			int getErrorCode() { return errorCode; }
			int getErrorColumn()	{ return errorColumn; }

			/*
				
			*/
			int lexicalAnalysis(const char *inStr, int len, int appended/*NOT USE*/, Token *tokens, int capability, int start);
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