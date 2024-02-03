#ifndef _NMAST_POOL_H
#define _NMAST_POOL_H

#ifdef _WIN32
#include <Windows.h>
#else

#endif

#include "common_data.h"

namespace nmath {
    static const int POOL_CAPACITY = 32;
    /*
        I use a pool to store AST node to reuse them later
        this reduces the number of allocation operation so it speed up the app
    */
    template <typename T>
    class NMASTPool {
    private:
        int gPoolSize = 0;
        NMAST<T>* AST_POOL[POOL_CAPACITY];
#ifdef _WIN32
        HANDLE poolMutex;
#else
        pthread_mutex_t poolMutex;
#endif
    public:
        NMASTPool() {
#ifdef _WIN32
            poolMutex = CreateMutex(
                NULL,              // default security attributes
                FALSE,             // initially not owned
                NULL);             // unnamed mutex
#else
            pthread_mutex_init(&poolMutex, nullptr);
#endif
        }

        virtual ~NMASTPool() {
            clear();
        }

        void clear() {
#ifdef _WIN32
            WaitForSingleObject(poolMutex,INFINITE);
#else
            pthread_mutex_lock(&poolMutex);
#endif
            if(gPoolSize > 0) {
                for(int i=0; i<gPoolSize; i++) {
                    free(AST_POOL[i]);
                    AST_POOL[i] = nullptr;
                }
                gPoolSize = 0;
            }
#ifdef _WIN32
#else
            pthread_mutex_unlock(&poolMutex);
#endif            
        }

        NMAST<T>* get() {
            NMAST<T> *node;

#ifdef _WIN32
            WaitForSingleObject(poolMutex,INFINITE);
#else
            pthread_mutex_lock(&poolMutex);
#endif
            if( gPoolSize > 0 ) {
                node = AST_POOL[gPoolSize-1];
                AST_POOL[gPoolSize-1] = nullptr;
                gPoolSize--;
#ifdef _WIN32
                // TODO: How to release in case of Windows?
#else
                pthread_mutex_unlock(&poolMutex);
#endif
            } else {
#ifdef _WIN32
                // TODO: How to release in case of Windows?
#else
                pthread_mutex_unlock(&poolMutex);
#endif
                node = new NMAST<T>;
                node->sign = 1;
                node->text = "0";
                node->left = node->right = node->parent = nullptr;
                node->value = 0;
                node->type = NUMBER;
                node->priority = 0;
            }
            return node;
        }

        /**
         * To be safe, when calling this method, please ensure that the node->left, node->right and node->parent
         * are nullptr
         * @param node
         */
        void put(NMAST<T> *node) {
            NMAST<T> *p;

            if(node == nullptr) return;

            if(node->left != nullptr) {
                NMAST<T> *l = node->left;
                l->parent = nullptr;
                node->left = nullptr;
                put(l);
            }

            if(node->right != nullptr) {
                NMAST<T> *r = node->right;
                r->parent = nullptr;
                node->right = nullptr;
                put(r);
            }

            if(node->parent != nullptr) {
                p = node->parent;
                if(p->left == node)
                    p->left = nullptr;
                else if(p->right == node)
                    p->right = nullptr;
                node->parent = nullptr;
            }

            node->sign = 1;
            node->text = "";
            node->left = node->right = node->parent = nullptr;
            node->value = 0;
            node->type = NUMBER;
            node->priority = 0;
#ifdef _WIN32
            WaitForSingleObject(poolMutex,INFINITE);
#else
            pthread_mutex_lock(&poolMutex);
#endif
            if( gPoolSize < POOL_CAPACITY ) {
                AST_POOL[gPoolSize] = node;
                gPoolSize++;
#ifdef _WIN32
#else
                pthread_mutex_unlock(&poolMutex);
#endif
                return;
            }
#ifdef _WIN32
#else
            pthread_mutex_unlock(&poolMutex);
#endif
            delete node;
        }
    };
}

#endif