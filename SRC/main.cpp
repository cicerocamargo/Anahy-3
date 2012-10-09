#include <iostream>
#include <stdlib.h>

using namespace std;

class Heap {
    
    int left(int parent);
    int right(int parent);
    int parent(int child);
    void heapifyup(int index);
    void heapifydown(int index);

public:
    
    int* heap;
    int _size, last_elem, pivo;
    
    Heap(int __size);
    ~Heap();
    
    void insert(int element);
    int deletemin();
        
    int size() { return last_elem; }
    int front();
    int back();
    int at(int ref);
    int* begin();
    int* end();
    
    void push_back(int elemento);
    void pop_back();
    void pop_front();
};

Heap::Heap(int __size) : _size(__size) {
    last_elem = 0;
    pivo = _size;
    heap = (int*) malloc(_size*sizeof(int));
}

Heap::~Heap() {
    
}

int Heap::front() {
    return heap[0];
}

int Heap::back() {
    return heap[last_elem];
}

int Heap::at(int ref) {
    return heap[ref];
}

int* Heap::begin() {
    heap;
}

int* Heap::end() {
    &heap[last_elem];
}

void Heap::push_back(int elemento) {
    int* more_numbers = NULL;
    int pivo = _size/2;
    
    pivo = pivo ? pivo : 1;
    
    if (last_elem == _size-1) {
        _size += pivo;
        more_numbers = (int*) realloc(heap, _size * sizeof(int));
    }
    if (more_numbers != NULL) {
        heap = more_numbers;    
    }
    
    heap[last_elem++] = elemento;
}

void Heap::pop_back() {
    int* elem;
    
    if (last_elem == 0) {
        return;
    }
    last_elem--;
}

void Heap::insert(int element) {
    push_back(element);
    heapifyup(size() - 1);
}

int Heap::deletemin() {
    int min = front();
    heap[0] = at(size() - 1);
    pop_back();
    heapifydown(0);
    return min;
}

void Heap::heapifyup(int index) {    

    while ( ( index > 0 ) && ( parent(index) >= 0 ) &&
            ( heap[parent(index)] > heap[index] ) ) {
        int tmp = heap[parent(index)];
        heap[parent(index)] = heap[index];
        heap[index] = tmp;
        index = parent(index);
    }
}

void Heap::heapifydown(int index) {     

    int child = left(index);
    if ( ( child > 0 ) && ( right(index) > 0 ) &&
         ( heap[child] > heap[right(index)] ) ) {
        child = right(index);
    }
    if ( child > 0 ) {
        int tmp = heap[index];
        heap[index] = heap[child];
        heap[child] = tmp;
        heapifydown(child);
    }
}

int Heap::left(int parent) {
    int i = ( parent << 1 ) + 1; // 2 * parent + 1
    return ( i < size() ) ? i : -1;
}

int Heap::right(int parent) {
    int i = ( parent << 1 ) + 2; // 2 * parent + 2
    return ( i < size() ) ? i : -1;
}

int Heap::parent(int child) {
    if (child != 0) {
        int i = (child - 1) >> 1;
        return i;
    }
    return -1;
}

int main() {
    // Create the heap
    Heap* myheap = new Heap(3);
    myheap->insert(700);
    myheap->insert(500);
    
    myheap->insert(100);
    
    myheap->insert(800);
    
    myheap->insert(200);
    
    myheap->insert(400);
    
    myheap->insert(900);
    
    myheap->insert(1000);
    
    myheap->insert(300);
    
    myheap->insert(600);
    


    // Get priority element from the heap
    int heapSize = myheap->size();
    for ( int i = 0; i < heapSize; i++ )
        cout << "Get min element = " << myheap->deletemin() << endl;

    // Cleanup
    delete myheap;
}
