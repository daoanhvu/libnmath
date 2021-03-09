#ifndef __BINARY_TREE_H_
#define __BINARY_TREE_H_

struct TNode {
	struct TNode *left;
	struct TNode *right;
	int key; 

	TNode() {
		this->left = nullptr;
		this->right = nullptr;
	}

	TNode(int k) {
		this->left = nullptr;
		this->right = nullptr;
		this->key = k;
	}
};
typedef struct TNode Node;
typedef struct TNode* PNode;

void addNode(PNode &tree, int newkey);
Node* findNode(const PNode &tree, int key);
void deleteNode(PNode &tree, int key);
void deleteAllTree(PNode &tree);
void inOrder(const PNode tree);
int height(const Node *tree);
void rebalance(PNode &tree);

#endif