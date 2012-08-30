#ifndef OURLIST_H
#define OURLIST_H

#include "Job.h"
#include "JobId.h"

template<class Type>
class Elem {
public:
	Elem(Type __element, Elem<Type>* __next) {//, Elem<Type>* __previous) {
		element = __element;
		next = __next;
		//previous = __previous;
	}

	Elem<Type>* next;
	//Elem<Type>* previous;
	Type element;
};

template<class Type>
class mList {
private:
	int __size;
	Elem<Type>* first;
	Elem<Type>* last;

public:
	mList() {
		__size = 0;
		first = last = NULL;
	}

	Type& front() {
		return first->element;
	}

	void pop_front() {
		Elem<Type>* e;
		if (__size == 0) {
			return;
		} else {
			e = first;
			first = first->next;
			//first->previous = first;
			if (__size == 1) {
				last = NULL;
			}
			delete e;
		}
		__size--;
	}

	void push_back(Type elem) {
		Elem<Type>* e = new Elem<Type>(elem, NULL);//, NULL);

		if (__size == 0) {
			first = e;
			last = e;
		} else if (__size == 1) {
			last = e;
			first->next = last;
			//last->previous = first;
		} else {
			//e->previous = last;
			last->next = e;
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