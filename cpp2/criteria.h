#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"


namespace nmath {
	enum NMathCClassType {
		SIMPLE, COMPOSITE
	};

	class Criteria {
		private:

		protected:
			char available;
			NMathCClassType cType;

		public:
			Criteria();
			virtual ~Criteria()		{}

			NMathCClassType getCClassType() const { return cType; }

			virtual Criteria* operator &(Criteria& c) { return this; }
			virtual Criteria* operator |(Criteria& c)	{ return this; }

			/* check if value does belong to this interval */
			virtual bool check(const double *value) { return false; }
			virtual Criteria* clone() { return 0; }
			virtual Criteria* getInterval(const float *values, int varCount) { return 0; }

			friend Criteria* buildTree(const NMAST *ast);
	};
}

#endif