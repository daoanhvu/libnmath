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
			NMASTList *mPrefix;
			NMASTList *mDomain;
			char mVariables[4];
			int mVarCount;

			int functionNotation(NLabLexer& lexer, int index);
			NMAST* buildIntervalTree(Token* valtk1, Token* o1, Token* variable, Token* o2, Token* valtk2);

		public:
			NLabParser();
			~NLabParser();
			void reset();

			NMASTList *prefix() { return mPrefix; }
			NMASTList *domain() { return mDomain; }
			int variableCount() { return mVarCount; }
			char* variables() const { return (char*)mVariables; }

			int getErrorColumn()	{ return errorColumn; }
			int getErrorCode() { return errorCode; }

			void parseExpression(NLabLexer& lexer, int *start);
			int parseFunctionExpression(NLabLexer& lexer);
			int parseDomain(NLabLexer& lexer, int *start);
	};
}

#endif