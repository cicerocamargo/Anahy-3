#ifndef _ANAHY_SMART_HEAP_H
#define _ANAHY_SMART_HEAP_H

#include <cstdlib>
#include <iostream>
#include "../definitions.h"

#define LEFT(i)     (i << 1)
#define RIGHT(i)    ((i << 1)+1)
#define PARENT(i)   (i >> 1)
#define SWAP(p,c)   { \
                        T tmp = heap[p]; \
                        heap[p] = heap[c]; \
                        heap[c] = tmp; \
                    }

#define POINTER_LT_OPERATOR(l,r) (*l < *r)
#define POINTER_GT_OPERATOR(l,r) (*l > *r)

template<class T>
class AnahySmartHeap {
    
    T* heap;
    uint _cur_size;
    uint _upper_bound;
    uint _lower_bound;
    uint _min_size;
    uint last_elem;
    
    void heapify_up(int child_index) {

        int parent_index = PARENT(child_index);
        while ( parent_index >= 1  && child_index > 1 && POINTER_GT_OPERATOR( heap[parent_index], heap[child_index] ) ) {
            SWAP(parent_index, child_index);
            child_index = PARENT(child_index);
            parent_index = PARENT(child_index);
        }
    }

    void heapify_down(int parent_index)  {
        
        int child_index = LEFT(parent_index);
        while (child_index <= _cur_size) {
            int r_child_index = RIGHT(parent_index);
            if ( (r_child_index <= _cur_size) && POINTER_GT_OPERATOR(heap[child_index],heap[r_child_index])) {
                child_index = r_child_index;
            }
            SWAP(parent_index, child_index);
            parent_index = child_index;
            child_index = LEFT(parent_index);
        }
    }

public:
        
    AnahySmartHeap(int size=64) {
        if (size < 12) {
            size = 12;
        }
        _cur_size = 0;
        _min_size = size;
        _upper_bound = size;
        _lower_bound = _upper_bound/4;
        last_elem = 1;
        heap = (T*) malloc(_upper_bound*sizeof(T));
    }
    
    ~AnahySmartHeap() {
        delete heap;
    }
    
    void insert(T element) {
        T* tmp = NULL;
        _cur_size++;
        
        if (_cur_size == _upper_bound) {
            
//std::cout << "Aumentando o heap!" << std::endl;
            _lower_bound *= 2;
            _upper_bound *= 2;
            tmp = (T*)realloc(heap, _upper_bound*sizeof(T));
            if (tmp) {
                heap = tmp;
            } else {
                std::cerr << "Error reallocating heap!" << std::endl;
                exit(1);
            }
        }
        
        heap[_cur_size] = element;
        heapify_up(_cur_size);
    }
    
    T extract_min() {
        if (_cur_size == 0)
        {
            return NULL;
        }

        T min = heap[1];
        heap[1] = heap[_cur_size];
        _cur_size--;
        if (_cur_size == _lower_bound && _cur_size > _min_size) { // realloc!
            
//std::cout << "Diminuindo o heap no AnahySmartHeap::extract_min!!" << std::endl;
            _upper_bound /= 2;
            _lower_bound /= 2;
            T* tmp = (T*)realloc(heap, _upper_bound*sizeof(T));
            if (tmp) {
                heap = tmp;
            } else {
                std::cerr << "Error reallocating heap!" << std::endl;
                exit(1);
            }
        }
        
        heapify_down(1);
        return min;
    }

    T extract_last() {
        if (_cur_size == 0)
        {
            return NULL;
        }
        
        T last = heap[_cur_size];
        _cur_size--;
        if (_cur_size == _lower_bound && _cur_size > _min_size) { // realloc!
            
//std::cout << "Diminuindo o heap no AnahySmartHeap::extract_last!" << std::endl;
            _upper_bound /= 2;
            _lower_bound /= 2;
            T* tmp = (T*)realloc(heap, _upper_bound*sizeof(T));
            if (tmp) {
                heap = tmp;
            } else {
                std::cerr << "Error reallocating heap!" << std::endl;
                exit(1);
            }
        }
        return last;
    }
        
    uint size() { return _cur_size; }

    bool empty() {
        if (size() == 0) 
            return true;
        else
            return false;
    }
    
    // // methods for display
    // void print_array() {
    //     
//std::cout << "[";
    //     for (int i = 1; i <= _cur_size; i++) {
    //         
//std::cout << heap[i]->get_id();
    //         if (i != _cur_size) {
    //             
//std::cout << ", ";
    //         }
    //     }
    //     
//std::cout << "]\n";
    // }
    
    // void print_tree(int parent=1, int num_tabs=0) {

    //     if (num_tabs == 0) {
    //         
//std::cout << "\n\n";
    //     }

    //     // print right tree
    //     if (RIGHT(parent) <= _cur_size) {
    //         print_tree(RIGHT(parent), num_tabs+1);
    //     }

    //     // print root
    //     for (int i=0; i<num_tabs; i++) {
    //         
//std::cout << "\t";
    //     }
    //     
//std::cout << heap[parent]->get_id() << std::endl;

    //     // print left tree
    //     if (LEFT(parent) <= _cur_size) {
    //         print_tree(LEFT(parent), num_tabs+1);
    //     }    

    //     if (num_tabs == 0) {
    //         
//std::cout << "\n\n";
    //     }
    // }
};

#endif