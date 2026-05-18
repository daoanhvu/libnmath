#ifndef _NLABPARSER_H
#define _NLABPARSER_H

#include "common.h"
#include "nlablexer.h"

namespace nmath {
	class NLabParser {
		private:
			int errorCode;
			int errorColumn;

			//output
			char mVariables[4];
			int mVarCount;

			int getType(const Token *t);
			
			int functionNotation(Token* tokens, int count, int index);
			NMAST* buildIntervalTree(Token* valtk1, Token* o1, Token* variable, Token* o2, Token* valtk2);

		public:
			NLabParser();
			~NLabParser();

			int variableCount() { return mVarCount; }
			char* variables() const { return (char*)mVariables; }
			int getErrorColumn()	{ return errorColumn; }
			int getErrorCode() { return errorCode; }

			int parseFunctionExpression(Token* tokens, int count, NMASTList *prefix, NMASTList *domain);
			NMAST* parseExpression(Token *tokens, int count, int *start);
			NMAST* parseDomain(Token *tokens, int count, int *start);
	};
}

#endif