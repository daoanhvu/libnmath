#ifndef _A_HPP_
#define _A_HPP_

namespace myns {
	template <typename T>
	class A {
	private:
		T value;
	public:
		virtual ~A() {};
		virtual int test() = 0;
	};
}

#endif