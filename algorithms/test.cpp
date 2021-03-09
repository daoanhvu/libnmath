#include<iostream>
#include "btree.h"
#include "binomialheap.hpp"

int main(int argc, char *args[]) {
	TNode *tree;

	addNode(tree, 23);
	addNode(tree, 100);
	addNode(tree, 50);
	addNode(tree, 37);
	addNode(tree, 76);
	addNode(tree, 80);

	int h = height(tree);
	std::cout << "Height: " << h << std::endl;

	inOrder(tree);

	std::cout << std::endl;
	deleteNode(tree, 76);
	h = height(tree);
	std::cout << "Height: " << h << std::endl;
	inOrder(tree);

	std::cout << std::endl;
	deleteNode(tree, 50);
	h = height(tree);
	std::cout << "Height: " << h << std::endl;
	inOrder(tree);

	std::cout << std::endl;
	deleteNode(tree, 80);
	h = height(tree);
	std::cout << "Height: " << h << std::endl;
	inOrder(tree);

	std::cout << std::endl;
	deleteNode(tree, 100);
	h = height(tree);
	std::cout << "Height: " << h << std::endl;
	inOrder(tree);

	std::cout << std::endl;
	deleteAllTree(tree);

	return 0;
}