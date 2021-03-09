#include "btree.h"
#include<iostream>

void addNode(PNode &tree, int newkey) {
	if(tree == nullptr) {
		Node *p = new Node();
		p->left = nullptr;
		p->right = nullptr;
		p->key = newkey;
		tree = p;
		return;
	}

	if(newkey < tree->key) {
		if(tree->left == nullptr) {
			Node *p = new Node();
			p->left = nullptr;
			p->right = nullptr;
			p->key = newkey;
			tree->left = p;
			return;
		}

		addNode(tree->left, newkey);
		rebalance(tree);
		return;
	}

	if(newkey > tree->key) {
		if(tree->right == nullptr) {
			Node *p = new Node();
			p->left = nullptr;
			p->right = nullptr;
			p->key = newkey;
			tree->right = p;
			return;
		}
		addNode(tree->right, newkey);
		rebalance(tree);
		return;
	}
}

void rebalance(PNode &tree) {
	if(tree == nullptr) {
		return;
	}

	int lh = height(tree->left);
	int rh = height(tree->right);

	if(std::abs(lh - rh) <= 1) {
		return;
	}

	Node *pp;
	if(lh > rh) {
		// Now we search for the maximum node in left child
		Node *pMax = tree->left;
		if(pMax->right != nullptr) {
			pp = pMax;
			pMax = pMax->right;

			// Re-balance the tree
			int temp = tree->key;
			tree->key = pMax->key;
			pp->right = pMax->left;
			tree->right = pMax;
			tree->right->key = temp;

			return;
		}

		pMax->right = tree;
		tree->left = nullptr;
		tree->right = nullptr;
		tree = pMax;

		return;
	}

	// rl > lh
	Node *pMin = tree->right;
	if(pMin->left != nullptr) {
		pp = pMin;
		pMin = pMin->left;

		// Re-balance the tree
		int tmp = tree->key;
		tree->key = pMin->key;
		pp->left = pMin->right;
		tree->left = pMin;
		tree->left->key = tmp;

		return;
	}

	pMin->left = tree;
	tree->left = nullptr;
	tree->right = nullptr;
	tree = pMin;
}

Node* findNode(const PNode &tree, int key) {
	if(tree == nullptr)
		return nullptr;

	if(key < tree->key) {
		return findNode(tree->left, key);
	}

	if(key > tree->key) {
		return findNode(tree->right, key);
	}

	return tree;
}

/**
	Handle the case that deletedNode has right child only.
	In such case, we search for the minimum key in right child of deleted node and 
	replace the deleted node with that node.
*/
void handleCaseDeletedNodeHasRightChildOnly(PNode &deletedNode) {
	Node *pMin = deletedNode->right;
	// Keep track the parent to swap later
	Node *pp = deletedNode;
	while(pMin->left != nullptr) {
		pp = pMin;
		pMin = pMin->left;
	}

	if(pp == deletedNode) {
		/* The right child of the deletedNode has no left child
			key = 20
				32                                 32
				/                                 /
			   /                                 /
			  20 pp, deletedNode   ===>         26
			   \                                 \
			    \                                 \ 
			     26 pMin                           27
			      \
			       \
			        27
		*/
		deletedNode->key = pMin->key;
		deletedNode->right = pMin->right;
		// Remove pMin from the tree and delete it
		pMin->right = nullptr;
		delete pMin;
		return;
	}

	/* The right child of deletedNode has left child, and we found the pMin
		key = 20
			32                                 32
			/                                 /
		   /                                 /
		  20 deletedNode   ===>             23
		   \                                 \
		    \                                 \ 
		     26 pp                            26
		     / \                             / \
		    /   \                           /   \
	pMin   23    27                        24    27
	        \
	         24
	*/
	pp->left = pMin->right;
	deletedNode->key = pMin->key;
	pMin->right = nullptr;
	delete pMin;
}

/*
	deletedNode has left child, we find the maximum key in left child
*/
void handleCaseDeletedNodeHasLeftChildOnly(PNode &deletedNode) {
	Node *pMax = deletedNode->left;
	// Keep track the parent to swap later
	Node *pp = deletedNode;
	while(pMax->right != nullptr) {
		pp = pMax;
		pMax = pMax->right;
	}

	if(pp == deletedNode) {
		/* Left child of deletedNode has no right child
			Example: key = 80
			   37                             37
			   /\                      ==>    /\
			 20  80(*) pp, deletednode      20  50
                 /                             /
               50 pMax	
               /		 
		*/
		deletedNode->key = pMax->key;
		deletedNode->left = pMax->left;
		pMax->left = nullptr;
		delete pMax;
		return;
	}

	/* Left child of deletedNode has right child, we search for the left most child
		Example: key = 80
		   37                             37
		   /\                      ==>    /\
		 20  80(*) deletednode          20  60
             /                             /
           50 pp                         50
           / \                           /\
              60 pMax                      55
              /
             55		 
	*/

	deletedNode->key = pMax->key;
	pp->right = pMax->left;
	pMax->left = nullptr;
	delete pMax;
}

/*
	In case the deleteNode has two children.
	We will compare the height of the two children
		- If left child is higher than right child -> search for maximum node of left child
		- If right child is higher than left child -> search for minimum node of right child
*/
void handleCaseDeletedNodeHasTwoChildren(PNode &deletedNode) {
	int lh = height(deletedNode->left);
	int rh = height(deletedNode->right);

	Node *pp = deletedNode;

	if(lh >= rh) {
		/* Left child higher than right child
			Example: key = 80
			   37                             37
			   /\                      ==>    /\
			 20  80(*) deletednode          20  60
	             /\                             /\
	       pp  50  100                        50 100
	           / \                           /\
	              60 pMax                      55
	              /
	             55		 
		*/
		Node *pMax = deletedNode->left;
		while(pMax->right != nullptr) {
			pp = pMax;
			pMax = pMax->right;
		}
		if(pp == deletedNode) {
			/* Left child of deletedNode has no right child
				Example: key = 80
				   37                             37
				   /\                      ==>    /\
				 20  80(*) pp, deletednode      20  50
	                 /\                             /\
	               50  100	                      45 100
	               /
	              45		 
			*/
			deletedNode->key = pMax->key;
			deletedNode->left = pMax->left;
			pMax->left = nullptr;
			delete pMax;
			return;
		}

		/* Left child of deletedNode has right child
			Example: key = 80
			   37                             37
			   /\                      ==>    /\
			 20  80(*)deletednode           20  55
                 /\                             /\
             pp 50  100	                      50 100
               /\                             /\
              45 55 pMax                    45  52
                 /
                52
		*/
		deletedNode->key = pMax->key;
		pp->right = pMax->left;
		pMax->left = nullptr;
		delete pMax;
		return;
	}

	// rh > lh
	Node *pMin = deletedNode->right;
	while(pMin->left != nullptr) {
		pp = pMin;
		pMin = pMin->left;
	}

	if(pp == deletedNode) {
		/* The right child of the deletedNode has no left child
			key = 20
				32                                 32
				/                                 /
			   /                                 /
			  20 pp, deletedNode   ===>         26
			  /\                               / \
			 /  \                             15  27 
		   15    26 pMin                           
			      \
			       27
		*/
		deletedNode->key = pMin->key;
		deletedNode->right = pMin->right;
		pMin->right = nullptr;
		delete pMin;
		return;
	}

	/* The right child of deletedNode has left child, and we found the pMin
		deleted key = 20
			32                                 32
			/                                 /
		   /                                 /
		  20 deletedNode   ===>             23
		  /\                               / \
		 /  \                             /   \ 
	    15    26 pp                     15     26
		     / \                             / \
		    /   \                           /   \
	pMin   23    27                        24    27
	        \
	         24
	*/
	pp->left = pMin->right;
	deletedNode->key = pMin->key;
	pMin->right = nullptr;
	delete pMin;
}

void deleteNode(PNode &tree, int key) {
	if(tree == nullptr) {
		return;
	}

	Node *deletedNode;

	if(key < tree->key) {
		Node *left = tree->left;
		if(left == nullptr) {
			return;
		}

		if(left->key == key) {
			deletedNode = left;

			if(deletedNode->left == nullptr) {
				// deletedNode has no left child
				if(deletedNode->right == nullptr) {
					// deletedNode is a leaf, just delete it
					tree->left = nullptr;
					delete deletedNode;
					return;
				}

				// deletedNode has right child only, we need to find the minimum value of the right child
				handleCaseDeletedNodeHasRightChildOnly(deletedNode);
				rebalance(tree);
				return;
			}

			// deletedNode has left child only, we're going to search for the maximum value of left child
			if(deletedNode->right == nullptr) {
				handleCaseDeletedNodeHasLeftChildOnly(deletedNode);
				return;
			}

			// deletedNode has two childrent, we're going to search for the maximum value of left child
			handleCaseDeletedNodeHasTwoChildren(deletedNode);
			return;
		}

		deleteNode(tree->left, key);
		return;
	}

	if(key > tree->key) {
		Node *right = tree->right;
		if(right == nullptr) {
			return;
		}

		if(right->key == key) {
			deletedNode = right;

			/* 
				Case 1: DeletedNode has no child
				Example: key = 80
				   37             37
				   /\      ==>    /
				 20  80(*)      20
			*/
			if(deletedNode->left == nullptr) {
				if(deletedNode->right == nullptr) {
					tree->right = nullptr;
					delete deletedNode;
					return;
				}

				// The deletedNode has right child only
				handleCaseDeletedNodeHasRightChildOnly(deletedNode);
				rebalance(tree);
				return;
			}

			// DeletedNode has left child only, we find the maximum key in left child
			if(deletedNode->right == nullptr) {
				handleCaseDeletedNodeHasLeftChildOnly(deletedNode);
				return;
			}

			// deletedNode has two childrent, we're going to search for the maximum value of left child
			handleCaseDeletedNodeHasTwoChildren(deletedNode);
			return;			
		}

		deleteNode(tree->right, key);
		return;
	}

	deletedNode = tree;
	// In case deletedNode has left child, we find the maximum key in left child
	if(deletedNode->left != nullptr) {
		Node *pMax = deletedNode->left;
		// Keep track the parent to swap later
		Node *pp = deletedNode;
		while(pMax->right != nullptr) {
			pp = pMax;
			pMax = pMax->right;
		}

		if(pp == deletedNode) {
			// Left child of deletedNode has only one node
			deletedNode->key = pMax->key;
			deletedNode->left = nullptr;
			delete pMax;
			rebalance(tree);
			return;
		}

		deletedNode->key = pMax->key;
		pp->right = pMax->left;
		pMax->left = nullptr;
		delete pMax;
		return;
	}

	// In case deletedNode has right child, we find the minimum key in right child
	if(deletedNode->right != nullptr) {
		Node *pMin = deletedNode->right;
		// Keep track the parent to swap later
		Node *pp = deletedNode;
		while(pMin->left != nullptr) {
			pp = pMin;
			pMin = pMin->left;
		}

		if(pp == deletedNode) {
			// The right child of deletedNode has no child
			deletedNode->key = pMin->key;
			deletedNode->right = nullptr;
			delete pMin;
			rebalance(tree);
			return;
		}

		// Now, we're going to move pMin
		pp->left = pMin->right;
		deletedNode->key = pMin->key;
		pMin->right = nullptr;
		delete pMin;
		return;
	}	

	// deletedNode has no child
	tree = nullptr;
	delete deletedNode;
}

void deleteAllTree(PNode &tree) {
	if(tree == nullptr) {
		return;
	}

	if(tree->left != nullptr) {
		deleteAllTree(tree->left);
	}

	if(tree->right != nullptr) {
		deleteAllTree(tree->right);
	}

	delete tree;
}

void inOrder(const PNode tree) {
	if(tree == nullptr) {
		return;
	}

	inOrder(tree->left);
	inOrder(tree->right);

	std::cout << " " << tree->key << " ";
}

int height(const Node *tree) {
	if(tree == nullptr) {
		return 0;
	}

	return 1 + std::max(height(tree->left), height(tree->right));
}
