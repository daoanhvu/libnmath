#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include "common.h"


namespace nmath {
	enum NMathCClassType {
		SIMPLE, COMBINED, COMPOSITE
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
			virtual int check(float *value) { return 0; }
	};
}

#endif