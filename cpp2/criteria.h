#ifndef __INTERVAL_H_
#define __INTERVAL_H_

#include <iostream>
#include "common.h"

using namespace std;

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

			virtual Criteria* andSelf(Criteria& c) { return 0; }

			virtual Criteria* operator &(Criteria& c) { return this; }
			virtual Criteria* operator |(Criteria& c)	{ return this; }

			friend istream& operator >>(istream& is, const Criteria& c);
			friend ostream& operator <<(ostream& os, const Criteria& c);

			/* check if value does belong to this interval */
			virtual bool check(const double *value) { return false; }
			virtual Criteria* clone() { return 0; }
			
			virtual bool containsVar(char var) { return false; }

			virtual Criteria* getInterval(const double *values, const char* var, int varCount) { return 0; }
			virtual Criteria* getIntervalF(const float *values, const char* var, int varCount) { return 0; }

			friend Criteria* buildCriteria(const NMAST *ast);
	};
}

#endif