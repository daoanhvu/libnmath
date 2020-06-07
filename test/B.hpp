#ifndef _B_HPP_
#define _B_HPP_

#include "A.hpp"

namespace myns {
	template <typename T>
	class B: public A<T> {
	private:
		size_t l;
	public:
		~B() {};
		int test();
	};

	template <typename T>
	int B<T>::test(){
		return 0;
	}
}

#endif