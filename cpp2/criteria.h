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
			bool available;
			NMathCClassType cType;

		public:
			Criteria();
			virtual ~Criteria()		{}

			bool isAvailable() { return available; }
			NMathCClassType getCClassType() const { return cType; }

			virtual Criteria* operator &(Criteria& c) { return this; }
			virtual Criteria* operator |(Criteria& c)	{ return this; }

			/* check if value does belong to this interval */
			virtual bool check(const double *value) { return false; }
			virtual Criteria* clone() { return 0; }

			template <class VT>
			Criteria* getInterval(const VT *values, const char* var, int varCount);

			friend Criteria* buildCriteria(const NMAST *ast);
	};
}

#endif