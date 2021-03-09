#ifndef __BINOMIAL_HEAP_H_
#define __BINOMIAL_HEAP_H_

#include<vector>

/**
 * @author D.A Vu
 * 
 */

/**
 * @brief Data structure for a node of the binomial tree
 * 
 * @tparam T A comparable type
 */
template <class T>
struct TagBNode {
    struct TagBNode<T> *parent;
    struct TagBNode<T> *child; // The first child in this list is the leftmost child
    struct TagBNode<T> *sibling; // Point to the right sibling
    int key;
    int degree;
    T data;

    TagBNode() {
        this->parent    = nullptr;
        this->child     = nullptr;
        this->sibling   = nullptr;
    }

    TagBNode(T data, int key) {
        this->parent    = nullptr;
        this->child     = nullptr;
        this->sibling   = nullptr;
        this->data = data;
        this->key = key;
    }
};
template <class T>
using BNode = struct TagBNode<T>;
template <class T>
using PBNode = struct TagBNode<T>*;

template <class T>
class BinomialHeap {
    private:
        std::vector<BNode<T>*> heap;

    public:
        BinomialHeap();

        BNode<T>* getNode(int index){ return this->heap[index]; }
        int listSize() { return heap.size(); }

        PBNode<T> minimum();
        void insert(PBNode<T> &node);
        void insert(T data, int key);
        PBNode<T> extractMin();
        void merge(BinomialHeap* &that);
        void decreaseKey(int key, int decreaseAmount);
        void unionHeap(BinomialHeap &that);
        friend BinomialHeap* unionHeaps(const BinomialHeap &bh1, const BinomialHeap &bh2);
        
};

template <class T>
BinomialHeap<T>::BinomialHeap() {
}

template <class T>
void BinomialHeap<T>::insert(PBNode<T> &node) {

}

template <class T>
void BinomialHeap<T>::insert(T data, int key) {
    BNode<T> *node = new BNode<T>(data, key);
    this->insert(node);
}

template <class T>
void BinomialHeap<T>::merge(BinomialHeap<T>* &that) {
    const int size1 = this->heap.size();
    const int size2 = that->listSize();
    int minSize = size1 > size2 ? size2 : size1;
    BNode<T> *node1;
    BNode<T> *node2;
    int i = 0;
    std::vector<BNode<T>*> heapTmp;
    while(i < minSize) {
        node1 = this->heap[i];
        node2 = that->getNode(i);

        if(node1->degree < node2->degree) {
            heapTmp.push_back(node1);
        } else if(node1->degree > node2->degree){
            heapTmp.push_back(node2);
        } else {
            if(node1->key > node2->key) {
                heapTmp.push_back(node1);
            } else {
                heapTmp.push_back(node2);
            }
        }
        i++;
    }

    if(size1 > size2) {
        for(i=size2; i<size1; i++) {
            heapTmp.push_back(node1);
        }
        return;
    }

    for(i=size1; i<size2; i++) {
        heapTmp.push_back(node2);
    }

    this->heap = heapTmp;
}

template <class T>
void BinomialHeap<T>::unionHeap(BinomialHeap<T> &that) {
    
}

template <class T>
BinomialHeap<T>* unionHeaps(const BinomialHeap<T> &bh1, const BinomialHeap<T> &bh2) {
    return nullptr;
}

#endif