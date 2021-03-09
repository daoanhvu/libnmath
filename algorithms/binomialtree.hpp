#ifndef __BINOMIAL_TREE_HPP_
#define __BINOMIAL_TREE_HPP_

#include<vector>

template <class Comparable>
struct TBinomialTree {
	std::vector<struct TBinomialTree*> children;
	Comparable value;
};
typedef struct TBinomialTree<int> BinomialTree;
typedef struct TBinomialTree<int>* PBinomialTree;

template <class Comparable>
void insertKey(PBinomialTree &tree, Comparable val);

#endif