#ifndef MLIST_H
#define MLIST_H

#include "Job.h"
#include "JobId.h"

template<class Type>
class Elem {
public:
	Elem(Type __element, Elem<Type>* __next) {
		element = __element;
		next = __next;
	}

	Elem<Type>* next;
	Type element;
};

template<class Type>
class mlist {
private:
	int __size;
	Elem<Type>* first;
	Elem<Type>* last;

public:
	mlist() {
		__size = 0;
		first = last = NULL;
	}

	Type& front() { return first->element; }

	Type& back() { return last->element; }

	void pop_front() {
		Elem<Type>* e;
		if (__size == 0) {
			return;
		} else {
			e = first;
			first = first->next;
		
			if (__size == 1) {
				last = NULL;
				first = NULL;
			}
			delete e;
			__size--;
		}
	}


	void push_back(Type elem) {
		Elem<Type>* e = new Elem<Type>(elem, NULL);

		if (__size == 0) {
			first = e;
			last = e;
		} else if (__size == 1) {
			last->next = e;
			e->next = e;
		} else {
			last->next = e;
			e->next = e; 
			last = e;
		}
		__size++;
	}

	int size() {
		return __size;
	}

	bool empty() {
		return __size == 0 ? true : false;
	}

};

#endif